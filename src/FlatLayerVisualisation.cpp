#include <glog/logging.h>
#include <GL/gl.h>

#include "FlatLayerVisualisation.hpp"

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

FlatLayerVisualisation::FlatLayerVisualisation(const fmri::LayerData &layer) :
        faceCount(layer.numEntries() * 4),
        vertexBuffer(faceCount * 3),
        colorBuffer(faceCount * 3),
        indexBuffer(faceCount * 3)
{
    auto& shape = layer.shape();
    CHECK_EQ(shape.size(), 2) << "layer should be flat!" << endl;
    CHECK_EQ(shape[0], 1) << "Only single images supported." << endl;

    const int limit = static_cast<const int>(layer.numEntries());
    auto data = layer.data();
    const auto [minElem, maxElem] = minmax_element(data, data + limit);

    auto scalingMax = max(abs(*minElem), abs(*maxElem));

    int v = 0;
    int j = 0;
    for (int i = 0; i < limit; ++i) {
        const float zOffset = -2 * i;
        const int vertexBase = i * 4;
        // Create the 4 vertices for the pyramid
        vertexBuffer[j++] = -0.5f;
        vertexBuffer[j++] = 0;
        vertexBuffer[j++] = 0.5f + zOffset;
        vertexBuffer[j++] = 0;
        vertexBuffer[j++] = 0;
        vertexBuffer[j++] = -0.5f + zOffset;
        vertexBuffer[j++] = 0;
        vertexBuffer[j++] = 1;
        vertexBuffer[j++] = 0 + zOffset;
        vertexBuffer[j++] = 0.5;
        vertexBuffer[j++] = 0;
        vertexBuffer[j++] = 0.5 + zOffset;

        // Define the colors for the vertices
        for (int c = 0; c < 4; ++c) {
            computeColor(data[i], scalingMax, &colorBuffer[12 * i + 3 * c]);
        }

        // Create the index set for the faces
        // Simply connect all vertices in ascending order and it works.
        indexBuffer[v++] = vertexBase;
        indexBuffer[v++] = vertexBase + 1;
        indexBuffer[v++] = vertexBase + 2;
        indexBuffer[v++] = vertexBase;
        indexBuffer[v++] = vertexBase + 1;
        indexBuffer[v++] = vertexBase + 3;
        indexBuffer[v++] = vertexBase;
        indexBuffer[v++] = vertexBase + 2;
        indexBuffer[v++] = vertexBase + 3;
        indexBuffer[v++] = vertexBase + 1;
        indexBuffer[v++] = vertexBase + 2;
        indexBuffer[v++] = vertexBase + 3;
    }
    assert(v == faceCount * 3);
    assert(j == faceCount * 3);
}

void FlatLayerVisualisation::render()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glColorPointer(3, GL_FLOAT, 0, colorBuffer.data());
    glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, indexBuffer.data());

    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_ARRAY);
}
