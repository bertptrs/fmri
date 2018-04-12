#pragma once

#include "utils.hpp"

namespace fmri
{

    /**
     * Base class for anything to be drawn to the screen.
     */
    class Drawable
    {
    public:
        virtual ~Drawable() = default;

        virtual void draw(float time) = 0;
        virtual void patchTransparancy();
        /**
         * Do any GL related initialization.
         *
         * GL initialization cannot be done in a separate thread, which
         * is where the constructor might be called.
         *
         * The default implementation does nothing.
         */
        virtual void glLoad();

    protected:
        std::vector<Color> colorBuffer;

        virtual float getAlpha() = 0;
    };

}
