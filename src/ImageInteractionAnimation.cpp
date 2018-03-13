#include "ImageInteractionAnimation.hpp"
#include "glutils.hpp"
#include "MultiImageVisualisation.hpp"
#include <caffe/util/math_functions.hpp>

using namespace fmri;


void ImageInteractionAnimation::draw(float step)
{
    auto vertexBuffer = deltas;
    caffe::caffe_scal(deltas.size(), step, vertexBuffer.data());
    caffe::caffe_add(vertexBuffer.size(), vertexBuffer.data(), startingPositions.data(), vertexBuffer.data());

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    texture.bind(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates.data());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glDrawArrays(GL_QUADS, 0, vertexBuffer.size() / 3);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

ImageInteractionAnimation::ImageInteractionAnimation(const DType *data, const std::vector<int> &shape,
                                                     const std::vector<float> &prevPositions,
                                                     const std::vector<float> &curPositions, float xDist) :
        texture(loadTexture(data, shape[2], shape[1] * shape[3], shape[1])),
        startingPositions(MultiImageVisualisation::getVertices(prevPositions)),
        deltas(MultiImageVisualisation::getVertices(curPositions)),
        textureCoordinates(MultiImageVisualisation::getTexCoords(shape[1]))
{
    caffe::caffe_sub(deltas.size(), deltas.data(), startingPositions.data(), deltas.data());

    for (auto i = 0u; i < deltas.size(); i += 3) {
        deltas[i] = xDist;
    }
}
