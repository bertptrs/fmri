#pragma once

#include <memory>

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
        std::unique_ptr<float[]> vertexBuffer;
        std::unique_ptr<float[]> colorBuffer;
        std::unique_ptr<int[]> indexBuffer;

        void setVertexPositions(int vertexNo, float* destination);
    };
}
