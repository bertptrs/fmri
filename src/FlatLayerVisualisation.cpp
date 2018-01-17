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

FlatLayerVisualisation::FlatLayerVisualisation(const LayerData &layer, Ordering ordering) :
        ordering(ordering),
        faceCount(layer.numEntries() * 4),
        vertexBuffer(new float[faceCount * 3]),
        colorBuffer(new float[faceCount * 3]),
        indexBuffer(new int[faceCount * 3])
{
    auto& shape = layer.shape();
    CHECK_EQ(shape.size(), 2) << "layer should be flat!" << endl;
    CHECK_EQ(shape[0], 1) << "Only single images supported." << endl;

    const auto limit = (int) layer.numEntries();
    auto data = layer.data();
    const auto [minElem, maxElem] = minmax_element(data, data + limit);

    auto scalingMax = max(abs(*minElem), abs(*maxElem));

    int v = 0;
    for (int i = 0; i < limit; ++i) {
        setVertexPositions(i, vertexBuffer.get() + 12 * i);
        const int vertexBase = i * 4;

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

void FlatLayerVisualisation::setVertexPositions(int vertexNo, float *destination)
{
    int j = 0;
    float zOffset;
    float yOffset;

    switch (ordering) {
        case Ordering::LINE:
            zOffset = -2 * vertexNo;
            yOffset = 0;
            break;

        case Ordering ::SQUARE:
            const auto nodes = faceCount / 4;
            auto columns = static_cast<int>(ceil(sqrt(nodes)));
            while (nodes % columns) ++columns;

            zOffset = -2 * (vertexNo % columns);
            yOffset = 2 * (vertexNo / columns);
            break;
    }

    // Create the 4 vertices for the pyramid
    destination[j++] = -0.5f;
    destination[j++] = 0 + yOffset; 
    destination[j++] = 0.5f + zOffset;
    destination[j++] = 0;
    destination[j++] = 0 + yOffset;
    destination[j++] = -0.5f + zOffset;
    destination[j++] = 0;
    destination[j++] = 1 + yOffset;
    destination[j++] = 0 + zOffset;
    destination[j++] = 0.5;
    destination[j++] = 0 + yOffset;
    destination[j++] = 0.5f + zOffset;
}
