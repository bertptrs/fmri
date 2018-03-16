#include <cmath>
#include <cstring>
#include <random>
#include <GL/gl.h>
#include <caffe/util/math_functions.hpp>
#include "Range.hpp"
#include "ActivityAnimation.hpp"

using namespace std;
using namespace fmri;

ActivityAnimation::Color ActivityAnimation::colorBySign(float intensity)
{
    if (intensity > 0) {
        return {0, 1, 0};
    } else {
        return {1, 0, 0};
    }
}

ActivityAnimation::ActivityAnimation(
        const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions, const float *aPositions,
        const float *bPositions, float xDist) :
        ActivityAnimation(interactions, aPositions, bPositions, xDist, ActivityAnimation::colorBySign)
{
}

ActivityAnimation::ActivityAnimation(
        const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
        const float *aPositions, const float *bPositions, float xDist, ColoringFunction coloring)
        :
        bufferLength(3 * interactions.size())
{
    CHECK(coloring) << "Invalid coloring function passed.";
    startingPos.reserve(bufferLength);
    delta.reserve(bufferLength);
    colorBuf.reserve(interactions.size());

    for (auto &entry : interactions) {
        auto *aPos = &aPositions[3 * entry.second.first];
        auto *bPos = &bPositions[3 * entry.second.second];

        colorBuf.push_back(coloring(entry.first));

        for (auto i : Range(3)) {
            startingPos.emplace_back(aPos[i]);
            delta.emplace_back(bPos[i] - aPos[i] + (i % 3 ? 0 : xDist));
        }
    }
}

void ActivityAnimation::draw(float timeScale)
{
    std::unique_ptr<float[]> vertexBuffer(new float[bufferLength]);
    caffe::caffe_copy(bufferLength, delta.data(), vertexBuffer.get());
    caffe::caffe_scal(bufferLength, timeScale, vertexBuffer.get());
    caffe::caffe_add(bufferLength, startingPos.data(), vertexBuffer.get(), vertexBuffer.get());

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colorBuf.data());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_POINTS, 0, bufferLength / 3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
