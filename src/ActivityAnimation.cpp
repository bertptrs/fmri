#include <cmath>
#include <cstring>
#include <random>
#include <GL/gl.h>
#include "Range.hpp"
#include "ActivityAnimation.hpp"
#include "utils.hpp"

using namespace fmri;

static inline float correct_timescale(float original)
{
    float ignore;
    return std::modf(original, &ignore);
}

ActivityAnimation::ActivityAnimation(std::size_t count, const float *aPos, const float *bPos, const float xDist) :
        bufferLength(3 * count),
        startingPos(new float[bufferLength]),
        delta(new float[bufferLength]),
        offset(new float[count])
{
    memcpy(startingPos.get(), aPos, sizeof(aPos[0]) * bufferLength);
    for (auto i : Range(bufferLength)) {
        delta[i] = bPos[i] - aPos[i];
    }

    auto& random = rng();
    std::uniform_real_distribution<float> rd;
    for (auto i : Range(count)) {
        offset[i] = rd(random);
        delta[3 * i] += xDist;
    }
}

void ActivityAnimation::draw(float timeScale) const
{
    std::unique_ptr<float[]> vertexBuffer(new float[bufferLength]);

    for (auto i : Range(bufferLength)) {
        vertexBuffer[i] = startingPos[i] + correct_timescale(offset[i/3] + timeScale) * delta[i];
    }

    glColor3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer.get());
    glDrawArrays(GL_POINTS, bufferLength / 3, bufferLength);
    glDisableClientState(GL_VERTEX_ARRAY);
}
