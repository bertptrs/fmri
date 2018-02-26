#pragma once

#include "LayerData.hpp"
#include "LayerVisualisation.hpp"
#include "Texture.hpp"

namespace fmri
{
    class InputLayerVisualisation : public LayerVisualisation
    {
    public:
        explicit InputLayerVisualisation(const LayerData &data);

        void render() override;

    private:
        Texture texture;
        float targetWidth;
        float targetHeight;

    };
}
