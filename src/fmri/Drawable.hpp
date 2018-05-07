#pragma once

#include <vector>
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
        virtual void patchTransparency();
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
        static constexpr auto BRAIN_SIZE = 15;

        std::vector<Color> colorBuffer;

        virtual float getAlpha() = 0;
        virtual void handleBrainMode(std::vector<float>& vertices);
        static bool brainModeEnabled();
    };

}
