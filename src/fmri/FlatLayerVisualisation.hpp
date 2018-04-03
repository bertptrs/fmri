#pragma once

#include <memory>

#include "LayerData.hpp"
#include "LayerVisualisation.hpp"

namespace fmri
{
    class FlatLayerVisualisation : public LayerVisualisation
    {
    public:
        explicit FlatLayerVisualisation(const LayerData &layer, Ordering ordering);

        void render() override;

    private:
        Ordering ordering;
        std::vector<float> vertexBuffer;
        std::vector<Color> colorBuffer;
        std::vector<int> indexBuffer;
        std::vector<int> activeIndexBuffer;

        static constexpr const std::array<float, 12> NODE_SHAPE = {
                -0.5f, 0, 0.5f,
                0, 0, -0.5f,
                0, 1, 0,
                0.5f, 0, 0.5f
        };
        static constexpr const std::array<int, 12> NODE_FACES = {
                0, 1, 2,
                0, 1, 3,
                0, 2, 3,
                1, 2, 3
        };
        static constexpr auto VERTICES_PER_NODE = std::tuple_size<typeof(NODE_SHAPE)>::value / 3;
        static constexpr auto FACES_PER_NODE = std::tuple_size<typeof(NODE_FACES)>::value / 3;

        void setVertexPositions(int vertexNo, float *destination);

        // Various functions defining the way the nodes will be aligned.
        void initializeNodePositions(std::size_t entries);
    };
}
