#include <glog/logging.h>
#include <GL/gl.h>

#include "FlatLayerVisualisation.hpp"
#include "Range.hpp"

using namespace std;
using namespace fmri;

static inline void computeColor(float intensity, float limit, float* destination)
{
    const float saturation = min(-log(abs(intensity) / limit) / 10.0f, 1.0f);
    if (intensity > 0) {
        destination[0] = saturation;
        destination[1] = saturation;
        destination[2] = 1;
    } else {
        destination[0] = 1;
        destination[1] = saturation;
        destination[2] = saturation;
    }
}

FlatLayerVisualisation::FlatLayerVisualisation(const LayerData &layer, Ordering ordering) :
        LayerVisualisation(layer.numEntries()),
        ordering(ordering),
        faceCount(layer.numEntries() * NODE_FACES.size() / 3),
        vertexBuffer(new float[faceCount * 3]),
        colorBuffer(new float[faceCount * 3]),
        indexBuffer(new int[faceCount * 3])
{
    auto& shape = layer.shape();
    CHECK_EQ(shape.size(), 2) << "layer should be flat!" << endl;
    CHECK_EQ(shape[0], 1) << "Only single images supported." << endl;

    initializeNodePositions();

    const auto limit = (int) layer.numEntries();
    auto data = layer.data();
    const auto [minElem, maxElem] = minmax_element(data, data + limit);

    auto scalingMax = max(abs(*minElem), abs(*maxElem));

    int v = 0;
    for (int i : Range(limit)) {
        setVertexPositions(i, vertexBuffer.get() + NODE_FACES.size() * i);
        const auto vertexBase = static_cast<int>(i * NODE_FACES.size() / 3);

        // Define the colors for the vertices
        for (auto c : Range(NODE_SHAPE.size() / 3)) {
            computeColor(data[i], scalingMax, &colorBuffer[NODE_FACES.size() * i + 3 * c]);
        }

        // Set the face nodes indices
        for (auto faceNode : NODE_FACES) {
            indexBuffer[v++] = vertexBase + faceNode;
        }
    }
    assert(v == (int) faceCount * 3);
}

void FlatLayerVisualisation::render()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glColorPointer(3, GL_FLOAT, 0, colorBuffer.get());
    glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, indexBuffer.get());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void FlatLayerVisualisation::setVertexPositions(const int vertexNo, float *destination)
{
    for (auto i : Range(NODE_SHAPE.size())) {
        destination[i] = NODE_SHAPE[i] + nodePositions_[3 * vertexNo + (i % 3)];
    }
}

void FlatLayerVisualisation::initializeNodePositions()
{
    switch (ordering) {
        case Ordering::LINE:
            computeNodePositionsLine();
            break;

        case Ordering::SQUARE:
            computeNodePositionsSquare();
            break;
    }
}

void FlatLayerVisualisation::computeNodePositionsSquare()
{
    const auto nodes = static_cast<int>(faceCount / 4);
    const auto columns = numCols(nodes);

    for (auto i : Range(nodes)) {
        nodePositions_[3 * i + 0] = 0;
        nodePositions_[3 * i + 1] = 2 * (i / columns);
        nodePositions_[3 * i + 2] = -2 * (i % columns);
    }
}

void FlatLayerVisualisation::computeNodePositionsLine()
{
    for (auto i : Range(static_cast<int>(faceCount / 4))) {
        nodePositions_[3 * i + 0] = 0;
        nodePositions_[3 * i + 1] = 0;
        nodePositions_[3 * i + 2] = -2.0f * i;
    }
}
