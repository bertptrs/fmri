#pragma once

#include "LayerVisualisation.hpp"
#include "LayerData.hpp"
#include "ActivityAnimation.hpp"
#include "LayerInfo.hpp"

namespace fmri {
    /**
     * Generate a static visualisation of a layer state.
     *
     * @param layer
     * @return A (possibly empty) visualisation. The caller is responsible for deallocating.
     */
    LayerVisualisation* getVisualisationForLayer(const LayerData& layer);

    ActivityAnimation *getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                            const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                            const vector<float> &curPositions);
}
