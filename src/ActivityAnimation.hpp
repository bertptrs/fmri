#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace fmri
{
    class ActivityAnimation
    {
    public:
        ActivityAnimation(std::size_t count, const float* aPos, const float* bPos, const float xDist);
        void draw(float timeScale) const;

    private:
        std::size_t bufferLength;
        std::vector<float> startingPos;
        std::vector<float> delta;
        std::vector<float> offset;
    };
}