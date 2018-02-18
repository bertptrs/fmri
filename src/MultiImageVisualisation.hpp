#pragma once

#include <GL/glew.h>
#include <map>
#include <memory>
#include "LayerVisualisation.hpp"
#include "LayerData.hpp"

namespace fmri
{
    class MultiImageVisualisation : public LayerVisualisation
    {
    public:
        explicit MultiImageVisualisation(const LayerData&);
        ~MultiImageVisualisation() override;

        void render() override;

    private:
        constexpr const static std::array<float, 12> BASE_VERTICES = {
                0, -1, -1,
                0, 1, -1,
                0, 1, 1,
                0, -1, 1,
        };
        std::vector<GLuint> textureReferences;
        std::unique_ptr<float[]> vertexBuffer;
    };
}
