#include <numeric>
#include <utility>
#include "visualisations.hpp"
#include "DummyLayerVisualisation.hpp"
#include "MultiImageVisualisation.hpp"
#include "FlatLayerVisualisation.hpp"
#include "Range.hpp"
#include "ActivityAnimation.hpp"

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

fmri::LayerVisualisation *fmri::getVisualisationForLayer(const fmri::LayerData &layer)
{
    switch (layer.shape().size()) {
        case 2:
            return new FlatLayerVisualisation(layer, FlatLayerVisualisation::Ordering::SQUARE);

        case 4:
            return new MultiImageVisualisation(layer);

        default:
            return new DummyLayerVisualisation();
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

    for (auto i : Range(numEntries)) {
        interactions[i] = weights[i] * data[i % shape[0]];
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

    return new ActivityAnimation(results, prevPositions.data(), curPositions.data(), -10);
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


        default:
            return nullptr;
    }
}
