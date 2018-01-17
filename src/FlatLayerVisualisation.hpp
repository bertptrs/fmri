#pragma once

#include <memory>

#include "LayerData.hpp"
#include "LayerVisualisation.hpp"

namespace fmri
{
    class FlatLayerVisualisation : public LayerVisualisation
    {
    public:
        enum class Ordering {
            LINE,
            SQUARE,
        };

        explicit FlatLayerVisualisation(const LayerData &layer, Ordering ordering);

        void render() override;

    private:
        Ordering ordering;
        std::size_t faceCount;
        std::unique_ptr<float[]> vertexBuffer;
        std::unique_ptr<float[]> colorBuffer;
        std::unique_ptr<int[]> indexBuffer;

        void setVertexPositions(const int vertexNo, float *destination);
    };
}
