#pragma once

#include <string>

namespace fmri
{
    /**
     * Singleton class defining the state of the current rendering.
     *
     * This class manages the currently loaded data and any related visualisations.
     */
    class RenderingState {
    public:
        /**
         * Reset the rendering state
         */
        void reset();
        void configureRenderingContext();
        void registerControls();
        /**
         * GLUT mouse handler function
         * @param x coordinate
         * @param y coordinate
         */
        void handleMouseAt(int x, int y);
        /**
         * GLUT keyboard handler function
         * @param x
         */
        void handleKey(unsigned char x);
        std::string infoLine();

        static RenderingState& instance();

    private:
        float pos[3];
        float angle[2];

        RenderingState() noexcept = default;

        void move(unsigned char key);
    };
}
