#include <caffe/util/math_functions.hpp>
#include "utils.hpp"

float fmri::LAYER_X_OFFSET = 10;

std::default_random_engine &fmri::rng()
{
    static std::default_random_engine rng;
    static std::default_random_engine::result_type seed = 0;

    if (seed == 0) {
        std::random_device dev;
        rng.seed(seed = dev());
    }

    return rng;
}

const std::vector<float> & fmri::animate(const std::vector<float> &start, const std::vector<float> &delta, float time)
{
    static std::vector<float> vertexBuffer;
    vertexBuffer = delta;
    caffe::caffe_scal(vertexBuffer.size(), time, vertexBuffer.data());
    caffe::caffe_add(vertexBuffer.size(), vertexBuffer.data(), start.data(), vertexBuffer.data());

    return vertexBuffer;
}
