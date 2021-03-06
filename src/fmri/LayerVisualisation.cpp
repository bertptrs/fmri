#include <GL/gl.h>
#include "LayerVisualisation.hpp"
#include "Range.hpp"
#include "utils.hpp"
#include "RenderingState.hpp"
#include "glutils.hpp"

const std::vector<float> &fmri::LayerVisualisation::nodePositions() const
{
    return nodePositions_;
}

fmri::LayerVisualisation::LayerVisualisation(size_t numNodes)
        : nodePositions_(numNodes * 3)
{
}

template<>
void fmri::LayerVisualisation::initNodePositions<fmri::LayerVisualisation::Ordering::LINE>(size_t n, float spacing)
{
    nodePositions_.clear();
    nodePositions_.reserve(3 * n);

    for (auto i : Range(n)) {
        nodePositions_.push_back(0);
        nodePositions_.push_back(0);
        nodePositions_.push_back(-spacing * i);
    }
}

float fmri::LayerVisualisation::getAlpha()
{
    return RenderingState::instance().layerAlpha();
}

void fmri::LayerVisualisation::setupLayerName(std::string_view name, fmri::LayerInfo::Type type)
{
    displayName = name;
    displayName += ": ";
    displayName += LayerInfo::nameByType(type);
}

void fmri::LayerVisualisation::drawLayerName() const
{
    glColor3f(0.5, 0.5, 0.5);
    renderText(displayName);

    glTranslatef(0, 0, -10);
}

template<>
void fmri::LayerVisualisation::initNodePositions<fmri::LayerVisualisation::Ordering::SQUARE>(size_t n, float spacing)
{
    nodePositions_.clear();
    nodePositions_.reserve(3 * n);
    const auto columns = numCols(n);

    for (auto i : Range(n)) {
        nodePositions_.push_back(0);
        nodePositions_.push_back(spacing * (i / columns));
        nodePositions_.push_back(-spacing * (i % columns));
    }
}
