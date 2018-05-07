#include <glog/logging.h>
#include <GL/gl.h>

#include "FlatLayerVisualisation.hpp"
#include "Range.hpp"
#include "RenderingState.hpp"

using namespace fmri;

static inline void computeColor(float intensity, float limit, Color& destination)
{
    const float saturation = FlatLayerVisualisation::intensityFunction(intensity, limit);
    if (saturation > 0) {
        destination = interpolate(saturation, POSITIVE_COLOR, NEUTRAL_COLOR);
    } else {
        destination = interpolate(-saturation, NEGATIVE_COLOR, NEUTRAL_COLOR);
    }
    if constexpr (alphaEnabled()) {
        // We have an alpha channel, set it to 1.
        destination[3] = 1;
    }
}

FlatLayerVisualisation::FlatLayerVisualisation(const LayerData &layer, Ordering ordering) :
        LayerVisualisation(layer.numEntries()),
        ordering(ordering),
        vertexBuffer(layer.numEntries() * NODE_FACES.size()),
        indexBuffer(layer.numEntries() * NODE_FACES.size())
{
    auto &shape = layer.shape();
    CHECK_EQ(shape.size(), 2) << "layer should be flat!\n";
    CHECK_EQ(shape[0], 1) << "Only single images supported.\n";

    initializeNodePositions(layer.numEntries());

    const auto limit = (int) layer.numEntries();
    auto data = layer.data();
    const auto
    [minElem, maxElem] = std::minmax_element(data, data + limit);

    auto scalingMax = std::max(abs(*minElem), abs(*maxElem));

    auto colorPos = std::back_inserter(colorBuffer);
    auto indexPos = indexBuffer.begin();

    for (int i : Range(limit)) {
        setVertexPositions(i, vertexBuffer.data() + NODE_FACES.size() * i);
        Color nodeColor;
        computeColor(data[i], scalingMax, nodeColor);
        colorPos = std::fill_n(colorPos, NODE_FACES.size() / 3, nodeColor);

        auto newIndexPos = std::copy(std::begin(NODE_FACES), std::end(NODE_FACES), indexPos);
        std::transform(indexPos, newIndexPos, indexPos, [i](auto x) { return x + i * VERTICES_PER_NODE;});
        indexPos = newIndexPos;
    }

    // Compute which nodes are active, add those to the active indices
    for (auto i : Range(limit)) {
        if (abs(data[i]) > EPSILON) {
            std::copy_n(&indexBuffer[NODE_FACES.size() * i], NODE_FACES.size(), std::back_inserter(activeIndexBuffer));
        }
    }

    assert(indexPos == indexBuffer.end());
    patchTransparency();
}

void FlatLayerVisualisation::draw(float)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    const auto& indices = RenderingState::instance().renderActivatedOnly() ? activeIndexBuffer : indexBuffer;

    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glColorPointer(std::tuple_size<Color>::value, GL_FLOAT, 0, colorBuffer.data());
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    glDisableClientState(GL_COLOR_ARRAY);

    // Now draw wireframe
    glColor4f(0, 0, 0, getAlpha());
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    glDisableClientState(GL_VERTEX_ARRAY);
}

void FlatLayerVisualisation::setVertexPositions(const int vertexNo, float *destination)
{
    for (auto i : Range(NODE_SHAPE.size())) {
        destination[i] = NODE_SHAPE[i] + nodePositions_[3 * vertexNo + (i % 3)];
    }
}

void FlatLayerVisualisation::initializeNodePositions(std::size_t entries)
{
    switch (ordering) {
        case Ordering::LINE:
            initNodePositions<Ordering::LINE>(entries, 2);
            break;

        case Ordering::SQUARE:
            initNodePositions<Ordering::SQUARE>(entries, 2);
            break;
    }
}

float FlatLayerVisualisation::intensityFunction(float f, float limit)
{
    if (abs(f) < EPSILON) {
        return 0;
    }

    const float magnitude = std::log(std::abs(f) / limit);
    const float result = std::clamp(1 + magnitude / 10.f, 0.f, 1.f);

    return std::copysign(result, f);
}
