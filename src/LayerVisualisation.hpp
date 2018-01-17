#pragma once

#include <vector>

namespace fmri
{
    class LayerVisualisation
    {
    public:
        LayerVisualisation() = default;
        LayerVisualisation(size_t numNodes);
        virtual ~LayerVisualisation() = default;

        virtual void render() = 0;
        virtual const std::vector<float>& nodePositions() const;

    protected:
        std::vector<float> nodePositions_;
    };
}
