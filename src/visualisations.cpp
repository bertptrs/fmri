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

    auto data = prevState.data();

    CHECK_GE(layer.parameters().size(), 1) << "Layer should have correct parameters";

    const auto shape = layer.parameters()[0]->shape();
    auto weights = layer.parameters()[0]->cpu_data();
    const auto numEntries = accumulate(shape.begin(), shape.end(), static_cast<size_t>(1), multiplies<void>());

    vector<float> interactions(numEntries);

    for (auto i : Range(numEntries)) {
        interactions[i] = weights[i] * data[i % shape[0]];
    }

    // Now use a creative argsort
    vector<size_t> idx(numEntries);
    iota(idx.begin(), idx.end(), 0);

    const auto desiredSize = min(INTERACTION_LIMIT, numEntries);
    nth_element(idx.begin(), idx.begin() + desiredSize, idx.end(), [&interactions](size_t a, size_t b) {
        return abs(interactions[a]) > abs(interactions[b]);
    });

    vector<Entry> result;
    result.reserve(desiredSize);
    for (auto i : Range(desiredSize)) {
        result.emplace_back(interactions[idx[i]], make_pair(idx[i] / shape[0], idx[i] % shape[0]));
    }

    return result;
}

Animation * fmri::getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerInfo &layer,
                                       const vector<float> &prevPositions, const vector<float> &curPositions)
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

    return new ActivityAnimation(entries, prevPositions.data(), curPositions.data(), -10);
}
