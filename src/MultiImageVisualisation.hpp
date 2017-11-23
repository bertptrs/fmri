#pragma once

#include <GL/glew.h>
#include <map>
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
        std::map<std::pair<int, int>, GLuint> textureReferences;
    };
}
