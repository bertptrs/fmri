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
static constexpr size_t INTERACTION_LIMIT = 1000;

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

static vector <pair<DType, pair<size_t, size_t>>>
computeActivityStrengths(const LayerData &prevState, const LayerInfo &layer)
{
    LOG(INFO) << "Computing top interactions for " << layer.name() << endl;

    typedef pair <DType, pair<size_t, size_t>> Entry;
    vector <Entry> result;

    auto data = prevState.data();

    CHECK_GE(layer.parameters().size(), 1) << "Layer should have correct parameters";

    const auto shape = layer.parameters()[0]->shape();
    auto weights = layer.parameters()[0]->cpu_data();
    const auto numEntries = accumulate(shape.begin(), shape.end(), 1u, multiplies<void>());
    result.reserve(numEntries);

    for (auto i : Range(numEntries)) {
        result.emplace_back(weights[i] * data[i % shape[0]], make_pair(i % shape[0], i / shape[0]));
    }

    const auto desiredSize = min(INTERACTION_LIMIT, result.size());
    partial_sort(result.begin(), result.begin() + desiredSize, result.end(), [](const Entry &a, const Entry &b) {
        return abs(a.first) > abs(b.first);
    });

    result.resize(desiredSize);

    return result;
}

Animation * fmri::getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                       const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                       const vector<float> &curPositions)
{
    if (layer.type() != LayerInfo::Type::InnerProduct) {
        // Only supported type at this time
        return nullptr;
    }

    if (prevPositions.empty() || curPositions.empty()) {
        // Not all positions know, no visualisation possible.
        return nullptr;
    }

    const auto entries = computeActivityStrengths(prevState, layer);

    return new ActivityAnimation(entries, prevPositions.data(), curPositions.data(), 2);
}
