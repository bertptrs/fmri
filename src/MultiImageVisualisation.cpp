#include <glog/logging.h>
#include "MultiImageVisualisation.hpp"
#include "glutils.hpp"

using namespace fmri;
using namespace std;

MultiImageVisualisation::MultiImageVisualisation(const fmri::LayerData &layer)
{
    auto dimensions = layer.shape();
    CHECK_EQ(4, dimensions.size()) << "Should be image-like layer";

    const auto images = dimensions[0],
            channels = dimensions[1],
            width = dimensions[2],
            height = dimensions[3];

    CHECK_EQ(1, images) << "Only single input image is supported" << endl;

    int columns = numCols(channels);

    // Create the quads for the images.
    int r = 0, c = 0;
    int v = 0;

    vertexBuffer = std::make_unique<float[]>(channels * 12);
    textureReferences.resize(channels);

    auto dataPtr = layer.data();

    for (int i = 0; i < channels; ++i) {
        textureReferences[i] = loadTexture(dataPtr, width, height);
        vertexBuffer[v++] = 0;
        vertexBuffer[v++] = 0 + 3 * r;
        vertexBuffer[v++] = 0 - 3 * c;

        vertexBuffer[v++] = 0;
        vertexBuffer[v++] = 2 + 3 * r;
        vertexBuffer[v++] = 0 - 3 * c;

        vertexBuffer[v++] = 0;
        vertexBuffer[v++] = 2 + 3 * r;
        vertexBuffer[v++] = 2 - 3 * c;

        vertexBuffer[v++] = 0;
        vertexBuffer[v++] = 0 + 3 * r;
        vertexBuffer[v++] = 2 - 3 * c;

        if (++c >= columns) {
            c = 0;
            ++r;
        }
        dataPtr += width * height;
    }
}

MultiImageVisualisation::~MultiImageVisualisation()
{
    for (auto entry : textureReferences) {
        glDeleteTextures(0, &entry);
    }
}

void MultiImageVisualisation::render()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor3f(0.3, 0.3, 0.3);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_QUADS, 0, 4 * textureReferences.size());
    glDisableClientState(GL_VERTEX_ARRAY);
}
