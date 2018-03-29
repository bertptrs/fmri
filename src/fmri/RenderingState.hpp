#pragma once

#include <string>
#include "LayerInfo.hpp"
#include "LayerData.hpp"
#include "LayerVisualisation.hpp"
#include "Animation.hpp"

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

        static RenderingState& instance();

    private:
        std::array<float, 3> pos;
        std::array<float, 2> angle;
        std::map<std::string, LayerInfo> layerInfo;
        std::vector<std::vector<LayerData>> layerData;
        std::vector<std::vector<LayerData>>::iterator currentData;
        std::vector<std::unique_ptr<LayerVisualisation>> layerVisualisations;
        std::vector<std::unique_ptr<Animation>> interactionAnimations;

        RenderingState() noexcept = default;

        void configureRenderingContext() const;

        void move(unsigned char key);
        void updateVisualisers();

        std::string infoLine()const;
        void renderDebugInfo() const;
        void renderLayerName(const std::string& name) const;
    };
}
