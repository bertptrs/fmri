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
    const int columns = numCols(channels);
    texture = loadTexture(layer.data(), width, channels * height, channels);
    for (auto i : Range(channels)) {
        nodePositions_[3 * i + 0] = 0;
        nodePositions_[3 * i + 1] = 3 * (i / columns);
        nodePositions_[3 * i + 2] = -3 * (i % columns);
    }

    vertexBuffer = std::make_unique<float[]>(channels * BASE_VERTICES.size());
    texCoordBuffer = std::make_unique<float[]>(channels * 2u * BASE_VERTICES.size() / 3);

    auto v = 0;

    for (auto i : Range(channels)) {
        const auto& nodePos = &nodePositions_[3 * i];
        for (auto j : Range(BASE_VERTICES.size())) {
            vertexBuffer[v++] = nodePos[j % 3] + BASE_VERTICES[j];
        }

        const float textureCoords[] = {
                1, (i + 1) / (float) channels,
                1, i / (float) channels,
                0, i / (float) channels,
                0, (i + 1) / (float) channels,
        };

        memcpy(texCoordBuffer.get() + 8 * i, textureCoords, sizeof(textureCoords));
    }
}

MultiImageVisualisation::~MultiImageVisualisation()
{
    glDeleteTextures(0, &texture);
}

void MultiImageVisualisation::render()
{
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordBuffer.get());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_QUADS, 0, nodePositions_.size() / 3 * 4);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
