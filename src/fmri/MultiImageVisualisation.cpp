#include <glog/logging.h>
#include "MultiImageVisualisation.hpp"
#include "glutils.hpp"
#include "Range.hpp"
#include "RenderingState.hpp"

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

    texture = loadTexture(layer.data(), width, channels * height, channels);
    initNodePositions<Ordering::SQUARE>(channels, 3);
    vertexBuffer = getVertices(nodePositions_);
    texCoordBuffer = getTexCoords(channels);
}

void MultiImageVisualisation::draw(float time)
{
    float alpha = RenderingState::instance().layerAlpha();
    drawImageTiles(vertexBuffer.size() / 3, vertexBuffer.data(), texCoordBuffer.data(), texture, alpha);
}

vector<float> MultiImageVisualisation::getVertices(const std::vector<float> &nodePositions, float scaling)
{
    std::vector<float> vertices;
    vertices.reserve(nodePositions.size() * BASE_VERTICES.size() / 3);
    for (auto i = 0u; i < nodePositions.size(); i += 3) {
        auto pos = &nodePositions[i];
        for (auto j = 0u; j < BASE_VERTICES.size(); ++j) {
            vertices.push_back(BASE_VERTICES[j] * scaling + pos[j % 3]);
        }
    }

    return vertices;
}

std::vector<float> MultiImageVisualisation::getTexCoords(int n)
{
    std::vector<float> coords;
    coords.reserve(8 * n);

    const float channels = n;

    for (int i = 0; i < n; ++i) {
        std::array<float, 8> textureCoords = {
                1, (i + 1) / channels,
                1, i / channels,
                0, i / channels,
                0, (i + 1) / channels,
        };

        for (auto coord : textureCoords) {
            coords.push_back(coord);
        }
    }

    return coords;
}
