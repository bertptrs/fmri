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

    typedef pair<DType, pair<size_t, size_t>> Entry;

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

    vector<Entry> result;
    result.reserve(desiredSize);
    for (auto i : idx) {
        result.emplace_back(interactions[i], make_pair(i / shape[0], i % shape[0]));
    }

    return new ActivityAnimation(result, prevPositions.data(), curPositions.data(), -10);
}

Animation * fmri::getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                       const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                       const vector<float> &curPositions)
{
    if (prevPositions.empty() || curPositions.empty()) {
        // Not all positions know, no visualisation possible.
        return nullptr;
    }


    switch (layer.type()) {
        case LayerInfo::Type::InnerProduct:
            return getFullyConnectedAnimation(prevState, layer,
                                              prevPositions, curPositions);

        default:
            return nullptr;
    }
}
