#include <glog/logging.h>
#include <cmath>
#include <caffe/util/math_functions.hpp>
#include "PoolingLayerAnimation.hpp"
#include "glutils.hpp"
#include "Range.hpp"
#include "MultiImageVisualisation.hpp"

using namespace std;
using namespace fmri;

PoolingLayerAnimation::PoolingLayerAnimation(const LayerData &prevData, const LayerData &curData,
                                             const std::vector<float> &prevPositions,
                                             const std::vector<float> &curPositions, float xDist) :
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
        deltas[i] = xDist;
    }
}

void PoolingLayerAnimation::draw(float timeStep)
{
    vector<float> vertexBuffer(deltas);
    caffe::caffe_scal(vertexBuffer.size(), timeStep, vertexBuffer.data());
    caffe::caffe_add(startingPositions.size(), startingPositions.data(), vertexBuffer.data(), vertexBuffer.data());

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    original.bind(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates.data());
    glDrawArrays(GL_QUADS, 0, vertexBuffer.size() / 3);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

Texture PoolingLayerAnimation::loadTextureForData(const LayerData &data)
{
    CHECK_EQ(data.shape().size(), 4) << "Layer should be image-like";
    CHECK_EQ(data.shape()[0], 1) << "Only single images supported";

    auto channels = data.shape()[1], width = data.shape()[2], height = data.shape()[3];
    return loadTexture(data.data(), width, height * channels, channels);
}
