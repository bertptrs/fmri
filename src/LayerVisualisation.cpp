#include "LayerVisualisation.hpp"

const std::vector<float> &fmri::LayerVisualisation::nodePositions() const
{
    return nodePositions_;
}

fmri::LayerVisualisation::LayerVisualisation(size_t numNodes)
        : nodePositions_(numNodes * 3)
{
}
