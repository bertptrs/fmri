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

    vertexBuffer = std::make_unique<float[]>(channels * 12);

    auto dataPtr = layer.data();
    for (auto i = 0; i < images; ++i) {
        for (auto j = 0; j < channels; ++j) {
            textureReferences[make_pair(i, j)] = loadTexture(dataPtr, width, height);
            dataPtr += width * height;
        }
    }

    int columns = sqrt(channels);
    while (channels % columns) columns++;
    int rows = channels / columns;

    // Create the quads for the images.
    int r = 0, c = 0;
    int v = 0;
    for (int i = 0; i < channels; ++i) {
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
    }
}

MultiImageVisualisation::~MultiImageVisualisation()
{
    for (auto entry : textureReferences) {
        glDeleteTextures(0, &entry.second);
    }
}

void MultiImageVisualisation::render()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor3f(1, 1, 1);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_QUADS, 0, 4 * textureReferences.size());
    glDisableClientState(GL_VERTEX_ARRAY);
}
