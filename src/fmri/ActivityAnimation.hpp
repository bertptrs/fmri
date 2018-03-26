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
        typedef std::array<float, 3> Color;
        typedef std::function<Color(float)> ColoringFunction;

        ActivityAnimation(
                    const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
                    const float *aPositions, const float *bPositions);
        ActivityAnimation(
                    const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
                    const float *aPositions, const float *bPositions, ColoringFunction coloring);
        void draw(float timeScale) override;

        static Color colorBySign(float intensity);

    private:
        std::size_t bufferLength;
        std::vector<std::array<float, 3>> colorBuf;
        std::vector<float> startingPos;
        std::vector<float> delta;
    };
}
