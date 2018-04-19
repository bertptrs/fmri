#pragma once

#include <string>
#include <future>
#include "LayerInfo.hpp"
#include "LayerData.hpp"
#include "LayerVisualisation.hpp"
#include "Animation.hpp"
#include "Options.hpp"

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
        /**
         * GLUT special keyboard handler.
         * @param key
         */
        void handleSpecialKey(int key);
        void render(float time) const;

        /**
         * Load rendering-specific options from the given options object.
         *
         * @param programOptions
         */
        void loadOptions(const Options& programOptions);
        /**
         * @return Whether the network should only render activated nodes, rather than all of them.
         */
        bool renderActivatedOnly() const;
        bool renderInteractionPaths() const;
        const Color& pathColor() const;
        float interactionAlpha() const;
        float layerAlpha() const;

        static RenderingState& instance();

    private:
        struct {
            bool showDebug = false;
            bool showHelp = true;
            bool renderLayers = true;
            bool renderInteractions = true;
            bool activatedOnly = false;
            bool renderInteractionPaths = false;
            float layerAlpha;
            float interactionAlpha;
            Color pathColor;
            bool mouse_1_pressed = false;
            bool mouse_2_pressed = false;
        } options;
        std::array<float, 3> pos;
        std::array<float, 2> angle;
        std::vector<std::vector<std::pair<std::unique_ptr<LayerVisualisation>, std::unique_ptr<Animation>>>> visualisations;
        std::future<decltype(visualisations)> loadingFuture;

        decltype(visualisations)::iterator currentData;


        RenderingState() noexcept;

        void configureRenderingContext() const;

        void move(unsigned char key, bool sprint);

        void idleFunc();

        std::string debugInfo() const;
        void renderOverlayText() const;
        void renderLayerName(const std::string& name) const;

        void drawLayer(float time, unsigned long i) const;

        void renderVisualisation(float time) const;

        void loadGLItems();

        bool isLoading() const;
    };
}
