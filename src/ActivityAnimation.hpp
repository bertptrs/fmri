#pragma once

#include <cstddef>
#include <memory>

namespace fmri
{
    class ActivityAnimation
    {
    public:
        ActivityAnimation(std::size_t count, const float* aPos, const float* bPos, const float xDist);
        void draw(float timeScale) const;

    private:
        std::size_t bufferLength;
        std::unique_ptr<float[]> startingPos;
        std::unique_ptr<float[]> delta;
        std::unique_ptr<float[]> offset;
    };
}