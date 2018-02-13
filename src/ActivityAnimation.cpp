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

    for (auto &entry : interactions) {
        auto *aPos = &aPositions[entry.second.first];
        auto *bPos = &bPositions[entry.second.second];

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

    glColor3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_POINTS, 0, bufferLength / 3);
    glDisableClientState(GL_VERTEX_ARRAY);
}
