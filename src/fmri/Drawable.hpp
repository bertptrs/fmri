#pragma once

#include "utils.hpp"

namespace fmri
{

    class Drawable
    {
    public:
        virtual ~Drawable() = default;

        virtual void draw(float time) = 0;
        virtual void patchTransparancy();
        virtual void glLoad();

    protected:
        std::vector<Color> colorBuffer;

        virtual float getAlpha() = 0;
    };

}
