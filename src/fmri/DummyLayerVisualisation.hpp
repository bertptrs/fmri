#pragma once

#include "LayerVisualisation.hpp"

namespace fmri
{
    /**
     * Visualisation that does not actually do anything.
     */
    class DummyLayerVisualisation : public LayerVisualisation
    {
    public:
        void draw(float time) override
        {};
    };
}
