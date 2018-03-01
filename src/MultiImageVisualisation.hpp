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

        void render() override;

    private:
        Texture texture;
        std::unique_ptr<float[]> vertexBuffer;
        std::unique_ptr<float[]> texCoordBuffer;
    };
}
