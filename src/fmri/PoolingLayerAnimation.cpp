#include <glog/logging.h>
#include <cmath>
#include <caffe/util/math_functions.hpp>
#include "PoolingLayerAnimation.hpp"
#include "glutils.hpp"
#include "MultiImageVisualisation.hpp"

using namespace std;
using namespace fmri;

PoolingLayerAnimation::PoolingLayerAnimation(const LayerData &prevData, const LayerData &curData,
                                             const std::vector<float> &prevPositions,
                                             const std::vector<float> &curPositions) :
        original(loadTextureForData(prevData)),
        downSampled(loadTextureForData(curData)),
        startingPositions(MultiImageVisualisation::getVertices(prevPositions)),
        deltas(startingPositions.size()),
        textureCoordinates(MultiImageVisualisation::getTexCoords(prevPositions.size() / 3))
{
    CHECK_EQ(prevPositions.size(), curPositions.size()) << "Layers should be same size. Caffe error?";
    const auto downScaling = sqrt(
            static_cast<float>(curData.shape()[2] * curData.shape()[3]) / (prevData.shape()[2] * prevData.shape()[3]));
    const auto targetPositions = MultiImageVisualisation::getVertices(curPositions, downScaling);
    caffe::caffe_sub(targetPositions.size(), targetPositions.data(), startingPositions.data(), deltas.data());

    for (auto i = 0u; i < deltas.size(); i+=3) {
        deltas[i] = LAYER_X_OFFSET;
    }
}

void PoolingLayerAnimation::draw(float timeStep)
{
    auto& vertexBuffer = animate(startingPositions, deltas, timeStep);

    drawImageTiles(vertexBuffer.size() / 3, vertexBuffer.data(), textureCoordinates.data(), original, getAlpha());
}

Texture PoolingLayerAnimation::loadTextureForData(const LayerData &data)
{
    CHECK_EQ(data.shape().size(), 4) << "Layer should be image-like";
    CHECK_EQ(data.shape()[0], 1) << "Only single images supported";

    auto channels = data.shape()[1], width = data.shape()[2], height = data.shape()[3];
    return Texture(data.data(), width, height * channels, GL_LUMINANCE, channels);
}

void PoolingLayerAnimation::glLoad()
{
    Drawable::glLoad();

    original.configure(GL_TEXTURE_2D);
    downSampled.configure(GL_TEXTURE_2D);
}
