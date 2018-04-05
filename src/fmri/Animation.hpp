#pragma once


#include "utils.hpp"

namespace fmri
{
    class Animation
    {
    public:
        virtual ~Animation() = default;

        virtual void draw(float step) = 0;

    private:
        static float getAlpha();

    protected:
        template<typename It>
        void patchTransparancy(It begin, It end)
        {
            if constexpr (std::tuple_size<Color>::value >= 4) {
                const auto alpha = getAlpha();
                for (; begin != end; ++begin) {
                    Color &color = *begin;
                    color[3] = alpha;
                }
            }
        }

    };
}
