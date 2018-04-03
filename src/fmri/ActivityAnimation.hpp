#pragma once

#include <cstddef>
#include <functional>
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
        std::vector<Color> colorBuf;
        std::vector<float> startingPos;
        std::vector<float> delta;
        std::vector<int> lineIndices;
    };
}
