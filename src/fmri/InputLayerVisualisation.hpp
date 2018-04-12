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

        void draw(float time) override;

        void glLoad() override;

    private:
        float targetWidth;
        float targetHeight;
        int width;
        int height;
        Texture texture;
        std::vector<float> textureBuffer;

    };
}
