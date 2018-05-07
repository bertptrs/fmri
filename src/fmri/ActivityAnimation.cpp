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

ActivityAnimation::ActivityAnimation(
            const std::vector<std::pair<DType, std::pair<std::size_t, std::size_t>>> &interactions,
            const float *aPositions, const float *bPositions) :
        bufferLength(3 * interactions.size()),
        delta(bufferLength)
{
    startingPos.reserve(bufferLength);
    vector<float> endPos;
    endPos.reserve(bufferLength);
    transform(interactions.begin(), interactions.end(), back_inserter(colorBuffer), [](auto e) {
        if (e.first > 0) {
            return interpolate(e.first, POSITIVE_COLOR, NEUTRAL_COLOR);
        } else {
            return interpolate(-e.first, NEGATIVE_COLOR, NEUTRAL_COLOR);
        }
    });
    colorBuffer.reserve(interactions.size());


    for (auto &entry : interactions) {
        auto *aPos = &aPositions[3 * entry.second.first];
        auto *bPos = &bPositions[3 * entry.second.second];

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

    patchTransparency();
}

void ActivityAnimation::draw(float timeScale)
{
    const auto &vertexBuffer = animate(startingPos, delta, timeScale);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(std::tuple_size<Color>::value, GL_FLOAT, 0, colorBuffer.data());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.data());
    glDrawArrays(GL_POINTS, 0, bufferLength / 3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ActivityAnimation::drawPaths()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    setGlColor(RenderingState::instance().pathColor());
    glVertexPointer(3, GL_FLOAT, 0, startingPos.data());
    glDrawElements(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, lineIndices.data());
    glDisableClientState(GL_VERTEX_ARRAY);
}
