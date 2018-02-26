#pragma once

#include "LayerVisualisation.hpp"
#include "LayerData.hpp"
#include "Animation.hpp"
#include "LayerInfo.hpp"

namespace fmri {
    /**
     * Generate a static visualisation of a layer state.
     *
     * @param data
     * @return A (possibly empty) visualisation. The caller is responsible for deallocating.
     */
    fmri::LayerVisualisation *getVisualisationForLayer(const fmri::LayerData &data, const fmri::LayerInfo &info);

    Animation * getActivityAnimation(const fmri::LayerData &prevState, const fmri::LayerData &curState,
                                     const fmri::LayerInfo &layer, const vector<float> &prevPositions,
                                     const vector<float> &curPositions);
}
