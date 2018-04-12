#pragma once

#include "Animation.hpp"
#include "utils.hpp"
#include "Texture.hpp"

namespace fmri
{
    class ImageInteractionAnimation : public Animation
    {
    public:
        ImageInteractionAnimation(const DType *data, const std::vector<int> &shape, const std::vector<float> &prevPositions,
                                          const std::vector<float> &curPositions);
        void draw(float step) override;
        void glLoad() override;

    private:
        Texture texture;
        std::vector<float> startingPositions;
        std::vector<float> deltas;
        std::vector<float> textureCoordinates;
    };
}
