#pragma once

#include "Animation.hpp"
#include "LayerData.hpp"
#include "Texture.hpp"

namespace fmri
{
    class PoolingLayerAnimation : public Animation
    {
    public:
        PoolingLayerAnimation(const LayerData &prevData, const LayerData &curData,
                                      const std::vector<float> &prevPositions,
                                      const std::vector<float> &curPositions);

        void draw(float timeStep) override;
        void glLoad() override;

    private:
        Texture original;
        Texture downSampled;
        std::vector<float> startingPositions;
        std::vector<float> deltas;
        std::vector<float> textureCoordinates;

        static Texture loadTextureForData(const LayerData& data);
    };
}
