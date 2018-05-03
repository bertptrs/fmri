#include <algorithm>
#include <numeric>
#include <caffe/util/math_functions.hpp>
#include <valarray>
#include "visualisations.hpp"
#include "DummyLayerVisualisation.hpp"
#include "MultiImageVisualisation.hpp"
#include "FlatLayerVisualisation.hpp"
#include "Range.hpp"
#include "ActivityAnimation.hpp"
#include "InputLayerVisualisation.hpp"
#include "PoolingLayerAnimation.hpp"
#include "ImageInteractionAnimation.hpp"

using namespace fmri;
using namespace std;

std::size_t fmri::INTERACTION_LIMIT = 10000;

typedef vector<pair<float, pair<size_t, size_t>>> EntryList;

/**
 * Normalizer for node positions.
 *
 * Since not every neuron in a layer may get a node in the visualisation,
 * this function maps those neurons back to a node number that does.
 *
 * Usage: node / getNodeNormalizer(layer).
 *
 * @param layer Layer to compute normalization for
 * @return Number to divide node numbers by.
 */
static inline int getNodeNormalizer(const LayerData& layer) {
    const auto& shape = layer.shape();
    switch(shape.size()) {
        case 2:
            return 1;

        case 4:
            return shape[2] * shape[3];

        default:
            CHECK(false) << "Unsupported shape " << shape.size() << endl;
            exit(EINVAL);
    }
}

/**
 * Deduplicate interaction entries.
 *
 * For duplicate interactions, the interaction strengths are summed.
 *
 * @param entries
 * @return the deduplicated entries.
 */
static EntryList deduplicate(const EntryList& entries)
{
    map<pair<size_t, size_t>, float> combiner;
    for (auto entry : entries) {
        combiner[entry.second] += entry.first;
    }

    EntryList result;
    transform(combiner.begin(), combiner.end(), back_inserter(result), [](const auto& item) {
        return make_pair(item.second, item.first);
    });

    return result;
}

static fmri::LayerVisualisation *getAppropriateLayer(const fmri::LayerData &data, const fmri::LayerInfo &info)
{
    switch (info.type()) {
        case LayerInfo::Type::Input:
            if (data.shape().size() == 4) {
                return new InputLayerVisualisation(data);
            } else {
                return new FlatLayerVisualisation(data, FlatLayerVisualisation::Ordering::SQUARE);
            }

        default:
            switch (data.shape().size()) {
                case 2:
                    return new FlatLayerVisualisation(data, FlatLayerVisualisation::Ordering::SQUARE);

                case 4:
                    return new MultiImageVisualisation(data);

                default:
                    return new DummyLayerVisualisation();
            }
    }
}

fmri::LayerVisualisation *fmri::getVisualisationForLayer(const fmri::LayerData &data, const fmri::LayerInfo &info)
{
    LOG(INFO) << "Loading state visualisation for " << data.name();
    auto layer = getAppropriateLayer(data, info);
    layer->setupLayerName(data.name(), info.type());

    return layer;
}

static Animation *getFullyConnectedAnimation(const fmri::LayerData &prevState, const fmri::LayerInfo &layer,
                                             const vector<float> &prevPositions, const vector<float> &curPositions)
{
    auto data = prevState.data();

    CHECK_GE(layer.parameters().size(), 1) << "Layer should have correct parameters";

    const auto shape = layer.parameters()[0]->shape();
    auto weights = layer.parameters()[0]->cpu_data();
    const auto numEntries = accumulate(shape.begin(), shape.end(), static_cast<size_t>(1), multiplies<void>());

    vector<float> interactions(numEntries);
    const auto stepSize = shape[1];

    for (auto i : Range(numEntries / stepSize)) {
        caffe::caffe_mul(shape[1], &weights[i * stepSize], data, &interactions[i * stepSize]);
    }

    const auto desiredSize = min(INTERACTION_LIMIT, numEntries);
    auto idx = arg_partial_sort(interactions.begin(), interactions.begin() + desiredSize, interactions.end(),
                                [](auto a, auto b) {
                                    return abs(a) > abs(b);
                                });

    EntryList result;
    result.reserve(desiredSize);
    const auto normalizer = getNodeNormalizer(prevState);
    for (auto i : idx) {
        if (abs(interactions[i]) < EPSILON){
            break;
        }
        result.emplace_back(interactions[i], make_pair((i % shape[1]) / normalizer, i / shape[1]));
    }

    return new ActivityAnimation(result, prevPositions.data(), curPositions.data());
}

static Animation *getDropOutAnimation(const fmri::LayerData &prevState,
                                      const fmri::LayerData &curState,
                                      const vector<float> &prevPositions,
                                      const vector<float> &curPositions) {
    const auto sourceNormalize = getNodeNormalizer(prevState);
    const auto sinkNormalize = getNodeNormalizer(curState);

    auto data = curState.data();
    EntryList results;
    results.reserve(curState.numEntries());
    for (auto i : Range(curState.numEntries())) {
        if (data[i] != 0) {
            results.emplace_back(data[i], make_pair(i / sourceNormalize, i / sinkNormalize));
        }
    }

    results = deduplicate(results);

    return new ActivityAnimation(results, prevPositions.data(), curPositions.data());
}

static Animation *getReLUAnimation(const fmri::LayerData &prevState,
                                   const fmri::LayerData &curState,
                                   const vector<float> &prevPositions,
                                   const vector<float> &curPositions) {
    CHECK_EQ(curState.numEntries(), prevState.numEntries()) << "Layers should be of same size!";

    std::vector<float> changes(prevState.numEntries());
    caffe::caffe_sub(prevState.numEntries(), curState.data(), prevState.data(), changes.data());

    if (curState.shape().size() == 2) {
        EntryList results;
        for (auto i : Range(curState.numEntries())) {
            if (curState.data()[i] > EPSILON) {
                results.emplace_back(changes[i], make_pair(i, i));
            }
        }

        return new ActivityAnimation(results, prevPositions.data(), curPositions.data());
    } else {
        return new ImageInteractionAnimation(changes.data(), prevState.shape(), prevPositions, curPositions);
    }
}

static Animation *getNormalizingAnimation(const fmri::LayerData &prevState, const LayerData &curState,
                                          const vector<float> &prevPositions,
                                          const vector<float> &curPositions) {
    CHECK(prevState.shape() == curState.shape()) << "Shapes should be of equal size" << endl;
    valarray<DType> scaling(prevState.data(), prevState.numEntries());
    scaling /= valarray<DType>(curState.data(), curState.numEntries());

    // Fix divisions by zero. For those cases, pick 1 since it doesn't matter anyway.
    normalize(begin(scaling), end(scaling));
    scaling = log(scaling);

    if (prevState.shape().size() == 2) {
        scaling /= scaling.max();
        EntryList entries;
        entries.reserve(scaling.size());
        for (auto i : Range(scaling.size())) {
            if (std::abs(curState[i]) > EPSILON) {
                entries.emplace_back(scaling[i], make_pair(i, i));
            }
        }
        return new ActivityAnimation(entries, prevPositions.data(), curPositions.data());

    } else {
        return new ImageInteractionAnimation(&scaling[0], prevState.shape(), prevPositions, curPositions);
    }
}

static Animation *getSoftmaxAnimation(const fmri::LayerData &curState, const vector<float> &prevPositions,
                                     const vector<float> &curPositions)
{
    CHECK_EQ(curState.shape().size(), 2) << "Softmax only supported for flat layers.";

    std::vector<float> intensities(curState.data(), curState.data() + curState.numEntries());
    rescale(intensities.begin(), intensities.end(), 0, 1);

    EntryList entries;
    for (auto i = 0u; i < intensities.size(); ++i) {
        entries.emplace_back(intensities[i], make_pair(i, i));
    }

    return new ActivityAnimation(entries, prevPositions.data(), curPositions.data());
}

Animation * fmri::getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                       const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                       const vector<float> &curPositions)
{
    if (prevPositions.empty() || curPositions.empty()) {
        // Not all positions known, no visualisation possible.
        return nullptr;
    }

    LOG(INFO) << "Loading interaction for " << layer.name();

    switch (layer.type()) {
        case LayerInfo::Type::InnerProduct:
            return getFullyConnectedAnimation(prevState, layer,
                                              prevPositions, curPositions);

        case LayerInfo::Type::DropOut:
            return getDropOutAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::ReLU:
            return getReLUAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::Pooling:
            return new PoolingLayerAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::LRN:
            return getNormalizingAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::Softmax:
            return getSoftmaxAnimation(curState, prevPositions, curPositions);

        default:
            return nullptr;
    }
}
