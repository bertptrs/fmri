#include <caffe/util/math_functions.hpp>
#include "utils.hpp"

float fmri::LAYER_X_OFFSET = 10;

fmri::Color fmri::NEUTRAL_COLOR = {1, 1, 1, 1};
fmri::Color fmri::NEGATIVE_COLOR = {1, 0, 0, 1};
fmri::Color fmri::POSITIVE_COLOR = {0, 0, 1, 1};

const std::vector<float> & fmri::animate(const std::vector<float> &start, const std::vector<float> &delta, float time)
{
    static std::vector<float> vertexBuffer;
    vertexBuffer = delta;
    caffe::caffe_scal(vertexBuffer.size(), time, vertexBuffer.data());
    caffe::caffe_add(vertexBuffer.size(), vertexBuffer.data(), start.data(), vertexBuffer.data());

    return vertexBuffer;
}
