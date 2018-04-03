#pragma once

#include <string>
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

        void loadSimulationData(const std::map<string, LayerInfo> &info, std::vector<std::vector<LayerData>> &&data);
        /**
         * Load rendering-specific options from the given options object.
         *
         * @param options
         */
        void loadOptions(const Options& options);
        /**
         * @return Whether the network should only render activated nodes, rather than all of them.
         */
        bool renderActivatedOnly() const;
        bool renderInteractionPaths() const;
        const Color& pathColor() const;

        static RenderingState& instance();

    private:
        struct {
            bool showDebug = false;
            bool showHelp = true;
            bool renderLayers = true;
            bool renderInteractions = true;
            bool activatedOnly = false;
            bool renderInteractionPaths = false;
            Color pathColor;
        } options;
        std::array<float, 3> pos;
        std::array<float, 2> angle;
        std::map<std::string, LayerInfo> layerInfo;
        std::vector<std::vector<LayerData>> layerData;
        std::vector<std::vector<LayerData>>::iterator currentData;
        std::vector<std::unique_ptr<LayerVisualisation>> layerVisualisations;
        std::vector<std::unique_ptr<Animation>> interactionAnimations;

        RenderingState() noexcept = default;

        void configureRenderingContext() const;

        void move(unsigned char key, bool sprint);
        void updateVisualisers();

        std::string debugInfo() const;
        void renderOverlayText() const;
        void renderLayerName(const std::string& name) const;
    };
}
