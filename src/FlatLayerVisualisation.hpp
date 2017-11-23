#pragma once

#include <vector>

#include "LayerData.hpp"
#include "LayerVisualisation.hpp"

namespace fmri
{
    class FlatLayerVisualisation : public LayerVisualisation
    {
    public:
        explicit FlatLayerVisualisation(const LayerData& layer);

        void render() override;

    private:
        std::size_t faceCount;
        std::vector<float> vertexBuffer;
        std::vector<float> colorBuffer;
        std::vector<int> indexBuffer;
    };
}
