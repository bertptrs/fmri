#include <algorithm>
#include <numeric>
#include <util/math_functions.hpp>
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

// Maximum number of interactions shown
static constexpr size_t INTERACTION_LIMIT = 10000;

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

fmri::LayerVisualisation *fmri::getVisualisationForLayer(const fmri::LayerData &data, const fmri::LayerInfo &info)
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

static Animation *getFullyConnectedAnimation(const fmri::LayerData &prevState, const fmri::LayerInfo &layer,
                                             const vector<float> &prevPositions, const vector<float> &curPositions)
{
    LOG(INFO) << "Computing top interactions for " << layer.name() << endl;

    auto data = prevState.data();

    CHECK_GE(layer.parameters().size(), 1) << "Layer should have correct parameters";

    const auto shape = layer.parameters()[0]->shape();
    auto weights = layer.parameters()[0]->cpu_data();
    const auto numEntries = accumulate(shape.begin(), shape.end(), static_cast<size_t>(1), multiplies<void>());

    vector<float> interactions(numEntries);
    const auto stepSize = shape[0];

    for (auto i : Range(numEntries / stepSize)) {
        caffe::caffe_mul(shape[0], &weights[i * stepSize], data, &interactions[i * stepSize]);
    }

    const auto desiredSize = min(INTERACTION_LIMIT, numEntries);
    auto idx = arg_nth_element(interactions.begin(), interactions.begin() + desiredSize, interactions.end(), [](auto a, auto b) {
        return abs(a) > abs(b);
    });

    EntryList result;
    result.reserve(desiredSize);
    const auto normalizer = getNodeNormalizer(prevState);
    for (auto i : idx) {
        result.emplace_back(interactions[i], make_pair(i / shape[0] / normalizer, i % shape[0]));
    }

    return new ActivityAnimation(result, prevPositions.data(), curPositions.data(), -10);
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

    return new ActivityAnimation(results, prevPositions.data(), curPositions.data(), -10);
}

static Animation *getReLUAnimation(const fmri::LayerData &prevState,
                                   const fmri::LayerData &curState,
                                   const vector<float> &prevPositions,
                                   const vector<float> &curPositions) {
    CHECK_EQ(curState.numEntries(), prevState.numEntries()) << "Layers should be of same size!";

    const auto prevData = prevState.data(), curData = curState.data();
    const auto sourceNormalize = getNodeNormalizer(prevState);
    const auto sinkNormalize = getNodeNormalizer(curState);

    EntryList results;

    for (auto i : Range(curState.numEntries())) {
        results.emplace_back(curData[i] - prevData[i], make_pair(i / sourceNormalize, i / sinkNormalize));
    }

    results = deduplicate(results);

    const auto maxValue = max_element(results.begin(), results.end())->first;

    return new ActivityAnimation(results, prevPositions.data(), curPositions.data(), -10, [=](float i) -> ActivityAnimation::Color {
        if (maxValue == 0) {
            return {1, 1, 1};
        } else {
            return {1 - i / maxValue, 1 - i / maxValue, 1};
        }
    });
}

static Animation *getNormalizingAnimation(const fmri::LayerData &prevState, const LayerData &curState,
                                          const vector<float> &prevPositions,
                                          const vector<float> &curPositions) {
    CHECK(prevState.shape() == curState.shape()) << "Shapes should be of equal size" << endl;
    vector<DType> scaling(std::accumulate(prevState.shape().begin(), prevState.shape().end(), 1u, multiplies<void>()));
    caffe::caffe_div(scaling.size(), prevState.data(), curState.data(), scaling.data());

    // Fix divisions by zero. For those cases, pick 1 since it doesn't matter anyway.
    normalize(scaling.begin(), scaling.end());

    if (prevState.shape().size() == 2) {
        EntryList entries;
        entries.reserve(scaling.size());
        for (auto i : Range(scaling.size())) {
            entries.emplace_back(scaling[i], make_pair(i, i));
        }

        auto max_val = *max_element(scaling.begin(), scaling.end());

        return new ActivityAnimation(entries, prevPositions.data(), curPositions.data(),-10,  [=](float i) -> ActivityAnimation::Color {
            auto intensity = clamp((i - 1) / (max_val - 1), 0.f, 1.f);
            return {
                    1 - intensity,
                    1,
                    1
            };
        });
    } else {
        transform(scaling.begin(), scaling.end(), scaling.begin(), [](float x) { return log(x); });
        return new ImageInteractionAnimation(scaling.data(), prevState.shape(), prevPositions, curPositions, -10);
    }
}

Animation * fmri::getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                       const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                       const vector<float> &curPositions)
{
    if (prevPositions.empty() || curPositions.empty()) {
        // Not all positions known, no visualisation possible.
        return nullptr;
    }


    switch (layer.type()) {
        case LayerInfo::Type::InnerProduct:
            return getFullyConnectedAnimation(prevState, layer,
                                              prevPositions, curPositions);

        case LayerInfo::Type::DropOut:
            return getDropOutAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::ReLU:
            return getReLUAnimation(prevState, curState, prevPositions, curPositions);

        case LayerInfo::Type::Pooling:
            return new PoolingLayerAnimation(prevState, curState, prevPositions, curPositions, -10);

        case LayerInfo::Type::LRN:
            return getNormalizingAnimation(prevState, curState, prevPositions, curPositions);

        default:
            return nullptr;
    }
}
