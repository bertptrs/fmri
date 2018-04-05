#include "ImageInteractionAnimation.hpp"
#include "glutils.hpp"
#include "MultiImageVisualisation.hpp"
#include "RenderingState.hpp"
#include <caffe/util/math_functions.hpp>

using namespace fmri;


void ImageInteractionAnimation::draw(float step)
{
    auto &vertexBuffer = animate(startingPositions, deltas, step);

    float alpha = RenderingState::instance().interactionAlpha();

    drawImageTiles(vertexBuffer.size() / 3, vertexBuffer.data(), textureCoordinates.data(), texture, alpha);
}

ImageInteractionAnimation::ImageInteractionAnimation(const DType *data, const std::vector<int> &shape, const std::vector<float> &prevPositions,
                                                     const std::vector<float> &curPositions) :
        texture(loadTexture(data, shape[2], shape[1] * shape[3], shape[1])),
        startingPositions(MultiImageVisualisation::getVertices(prevPositions)),
        deltas(MultiImageVisualisation::getVertices(curPositions)),
        textureCoordinates(MultiImageVisualisation::getTexCoords(shape[1]))
{
    caffe::caffe_sub(deltas.size(), deltas.data(), startingPositions.data(), deltas.data());

    for (auto i = 0u; i < deltas.size(); i += 3) {
        deltas[i] = LAYER_X_OFFSET;
    }
}
