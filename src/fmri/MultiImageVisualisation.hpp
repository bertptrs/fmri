#pragma once

#include <map>
#include <memory>
#include "LayerVisualisation.hpp"
#include "LayerData.hpp"
#include "Texture.hpp"

namespace fmri
{
    class MultiImageVisualisation : public LayerVisualisation
    {
    public:
        constexpr const static std::array<float, 12> BASE_VERTICES = {
                0, -1, -1,
                0, 1, -1,
                0, 1, 1,
                0, -1, 1,
        };

        explicit MultiImageVisualisation(const LayerData&);

        void draw(float time) override;

        void glLoad() override;

        static vector<float> getVertices(const std::vector<float> &nodePositions, float scaling = 1);
        static std::vector<float> getTexCoords(int n);

    private:
        Texture texture;
        std::vector<float> vertexBuffer;
        std::vector<float> texCoordBuffer;
    };
}
