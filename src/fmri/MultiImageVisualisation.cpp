#include <glog/logging.h>
#include "MultiImageVisualisation.hpp"
#include "glutils.hpp"
#include "Range.hpp"

using namespace fmri;
using namespace std;

MultiImageVisualisation::MultiImageVisualisation(const fmri::LayerData &layer) :
    texture(layer.data(), layer.shape().at(2), layer.shape().at(3) * layer.shape().at(1), GL_LUMINANCE, layer.shape().at(1))
{
    auto dimensions = layer.shape();

    const auto images = dimensions[0],
            channels = dimensions[1];

    CHECK_EQ(1, images) << "Only single input image is supported" << endl;

    initNodePositions<Ordering::SQUARE>(channels, 3);
    vertexBuffer = getVertices(nodePositions_);
    texCoordBuffer = getTexCoords(channels);

    handleBrainMode(vertexBuffer);
    handleBrainMode(nodePositions_);
}

void MultiImageVisualisation::draw(float)
{
    drawImageTiles(vertexBuffer.size() / 3, vertexBuffer.data(), texCoordBuffer.data(), texture, getAlpha());
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

void MultiImageVisualisation::glLoad()
{
    Drawable::glLoad();

    texture.configure(GL_TEXTURE_2D);
}
