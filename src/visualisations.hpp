#pragma once

#include "LayerVisualisation.hpp"
#include "LayerData.hpp"

namespace fmri {
    /**
     * Generate a static visualisation of a layer state.
     *
     * @param layer
     * @return A (possibly empty) visualisation. The caller is responsible for deallocating.
     */
    LayerVisualisation* getVisualisationForLayer(const LayerData& layer);
}