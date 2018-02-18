#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include "Animation.hpp"
#include "utils.hpp"

namespace fmri
{
    class ActivityAnimation
            : public Animation
    {
    public:
        ActivityAnimation(const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
                                  const float *aPositions, const float *bPositions, float xDist);
        void draw(float timeScale) override;

    private:
        std::size_t bufferLength;
        std::vector<std::array<float, 3>> colorBuf;
        std::vector<float> startingPos;
        std::vector<float> delta;
    };
}