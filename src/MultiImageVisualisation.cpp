#include <glog/logging.h>
#include "MultiImageVisualisation.hpp"
#include "glutils.hpp"
#include "Range.hpp"

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

    nodePositions_.resize(channels * 3);
    auto dataPtr = layer.data();
    const int columns = numCols(channels);
    textureReferences.resize(channels);
    for (auto i : Range(channels)) {
        textureReferences[i] = loadTexture(dataPtr, width, height);
        dataPtr += width * height;

        nodePositions_[3 * i + 0] = 0;
        nodePositions_[3 * i + 1] = 3 * (i / columns);
        nodePositions_[3 * i + 2] = -3 * (i % columns);
    }

    vertexBuffer = std::make_unique<float[]>(channels * BASE_VERTICES.size());

    auto v = 0;

    for (auto i : Range(channels)) {
        const auto& nodePos = &nodePositions_[3 * i];
        for (auto j : Range(BASE_VERTICES.size())) {
            vertexBuffer[v++] = nodePos[j % 3] + BASE_VERTICES[j];
        }
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
