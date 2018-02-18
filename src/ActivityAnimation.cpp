#include <cmath>
#include <cstring>
#include <random>
#include <GL/gl.h>
#include "Range.hpp"
#include "ActivityAnimation.hpp"

using namespace std;
using namespace fmri;

ActivityAnimation::ActivityAnimation(const vector<pair<DType, pair<size_t, size_t>>> &interactions,
                                     const float *aPositions, const float *bPositions, float xDist) :
        bufferLength(3 * interactions.size())
{
    startingPos.reserve(bufferLength);
    delta.reserve(bufferLength);
    colorBuf.reserve(interactions.size());

    for (auto &entry : interactions) {
        auto *aPos = &aPositions[3 * entry.second.first];
        auto *bPos = &bPositions[3 * entry.second.second];

        array<float, 3> color;
        if (entry.first > 0) {
            color = {0, 1, 0};
        } else {
            color = {1, 0, 0};
        }
        colorBuf.push_back(color);

        for (auto i : Range(3)) {
            startingPos.emplace_back(aPos[i]);
            delta.emplace_back(bPos[i] - aPos[i] + (i % 3 ? 0 : xDist));
        }
    }
}

void ActivityAnimation::draw(float timeScale)
{
    std::unique_ptr<float[]> vertexBuffer(new float[bufferLength]);

    for (auto i : Range(bufferLength)) {
        vertexBuffer[i] = startingPos[i] + timeScale * delta[i];
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colorBuf.data());
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_POINTS, 0, bufferLength / 3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
