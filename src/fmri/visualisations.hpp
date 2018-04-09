#pragma once

#include "LayerVisualisation.hpp"
#include "LayerData.hpp"
#include "Animation.hpp"
#include "LayerInfo.hpp"

namespace fmri {
    /**
     * Maximum number of interactions per layer.
     *
     * Controls the number of interactions per layer for performance
     * reasons. Only the top INTERACTION_LIMIT interactions are shown
     * to limit the amount of computations needed for animation.
     *
     * Note that this number currently applies only to InnerProduct
     * type layers.
     *
     * This value is set from the options parser.
     */
    extern std::size_t INTERACTION_LIMIT;

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
