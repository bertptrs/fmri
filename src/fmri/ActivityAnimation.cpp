#include <cmath>
#include <cstring>
#include <random>
#include <GL/gl.h>
#include <caffe/util/math_functions.hpp>
#include "Range.hpp"
#include "ActivityAnimation.hpp"
#include "RenderingState.hpp"
#include "glutils.hpp"

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
            const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
            const float *aPositions, const float *bPositions) :
        ActivityAnimation(interactions, aPositions, bPositions, ActivityAnimation::colorBySign)
{
}

ActivityAnimation::ActivityAnimation(
            const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
            const float *aPositions, const float *bPositions, ColoringFunction coloring)
        :
        bufferLength(3 * interactions.size()),
        delta(bufferLength)
{
    CHECK(coloring) << "Invalid coloring function passed.";
    startingPos.reserve(bufferLength);
    vector<float> endPos;
    endPos.reserve(bufferLength);
    colorBuf.reserve(interactions.size());

    for (auto &entry : interactions) {
        auto *aPos = &aPositions[3 * entry.second.first];
        auto *bPos = &bPositions[3 * entry.second.second];

        colorBuf.push_back(coloring(entry.first));

        for (auto i : Range(3)) {
            startingPos.emplace_back(aPos[i]);
            endPos.emplace_back(bPos[i] + (i % 3 ? 0 : LAYER_X_OFFSET));
        }
    }

    caffe::caffe_sub(endPos.size(), endPos.data(), startingPos.data(), delta.data());
    startingPos.insert(startingPos.end(), endPos.begin(), endPos.end());
    for (auto i : Range(interactions.size())) {
        lineIndices.push_back(i);
        lineIndices.push_back(i + interactions.size());
    }
}

void ActivityAnimation::draw(float timeScale)
{
    const auto &vertexBuffer = animate(startingPos, delta, timeScale);
    glPointSize(5);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colorBuf.data());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glDrawArrays(GL_POINTS, 0, bufferLength / 3);
    glDisableClientState(GL_COLOR_ARRAY);
    if (RenderingState::instance().renderInteractionPaths()) {
        glColor4f(1, 1, 1, 0.1);
        glVertexPointer(3, GL_FLOAT, 0, startingPos.data());
        glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, lineIndices.data());
        checkGLErrors();
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}
