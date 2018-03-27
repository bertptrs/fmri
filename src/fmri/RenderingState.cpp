#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include "RenderingState.hpp"
#include "utils.hpp"

using namespace fmri;

static RenderingState& state = RenderingState::instance();

/**
 * Static utility function to bind to GLUT.
 *
 * @param x
 * @param y
 */
static void handleMouseMove(int x, int y)
{
    state.handleMouseAt(x, y);
}

/**
 * Static utility function to bind to GLUT.
 *
 * @param key
 */
static void handleKeys(unsigned char key, int, int)
{
    state.handleKey(key);
}

static float getFPS()
{
    static int frames = 0;
    static float fps = 0;
    static auto timeBase = glutGet(GLUT_ELAPSED_TIME);

    ++frames;
    const auto time = glutGet(GLUT_ELAPSED_TIME);
    if (time - timeBase > 2000) {
        fps = frames * 1000.0f / (time - timeBase);
        frames = 0;
        timeBase = time;
    }


    return fps;
}

void RenderingState::move(unsigned char key)
{
    float speed = 0.5;
    float dir[3];

    const auto yaw = deg2rad(state.angle[0]);
    const auto pitch = deg2rad(state.angle[1]);

    if (key == 'w' || key == 's') {
        dir[0] = std::sin(yaw) * std::cos(pitch);
        dir[1] = -std::sin(pitch);
        dir[2] = -std::cos(yaw) * std::cos(pitch);
    } else {
        dir[0] = -std::cos(yaw);
        dir[1] = 0;
        dir[2] = -std::sin(yaw);
    }

    if (key == 's' || key == 'd') {
        speed *= -1;
    }

    for (auto i = 0; i < 3; ++i) {
        pos[i] += speed * dir[i];
    }
}

void RenderingState::handleKey(unsigned char x)
{
    switch (x) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            move(x);
            break;

        case 'q':
            exit(0);

        case 'h':
            state.reset();
            break;

        default:
            // Do nothing.
            break;
    }

    glutPostRedisplay();
}

std::string RenderingState::infoLine()
{
    std::stringstream buffer;
    buffer << "Pos(x,y,z) = (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
    buffer << "Angle(p,y) = (" << angle[0] << ", " << angle[1] << ")\n";
    buffer << "FPS = " << getFPS() << "\n";
    return buffer.str();
}

void RenderingState::reset()
{
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 3;

    angle[0] = 0;
    angle[1] = 0;
}

void RenderingState::configureRenderingContext()
{
    glLoadIdentity();
    glRotatef(angle[1], 1, 0, 0);
    glRotatef(angle[0], 0, 1, 0);
    glTranslatef(-pos[0], -pos[1], -pos[2]);
}

RenderingState &RenderingState::instance()
{
    static RenderingState state;
    return state;
}

void RenderingState::registerControls()
{
    reset();
    glutPassiveMotionFunc(handleMouseMove);
    glutKeyboardFunc(handleKeys);
}

void RenderingState::handleMouseAt(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2.f;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2.f;

    angle[0] = (x - width) / width * 180;
    angle[1] = (y - height) / height * 90;

    glutPostRedisplay();
}
