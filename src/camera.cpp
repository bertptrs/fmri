#include <GL/glut.h>
#include <cmath>
#include "camera.hpp"

using namespace fmri;
using namespace std;

static float yaw, pitch;

static float pos[3];

static void handleMouseMove(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    yaw = (x - width) / width * 180;
    pitch = (y - height) / height * 90;
}

static void move(unsigned char key)
{
    float speed = 0.2;
    float dir[3];
    // Currently very buggy
    if (key == 'w' || key == 's') {
        dir[0] = 0;
        dir[1] = 0;
        dir[2] = -1;
    } else {
        dir[0] = -1;
        dir[1] = 0;
        dir[2] = 0;
    }

    if (key == 's' || key == 'd') {
        speed *= -1;
    }

    for (unsigned int i = 0; i < 3; ++i) {
        pos[i] += speed * dir[i];
    }
}

static void handleKeys(unsigned char key, int, int)
{
    switch (key) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            move(key);
            break;

        case 'q':
            // Utility quit function.
            exit(0);

        default:
            // Do nothing.
            break;
    }
}

void fmri::registerCameraControls()
{
    resetCamera();
    glutPassiveMotionFunc(handleMouseMove);
    glutKeyboardFunc(handleKeys);
}

void fmri::resetCamera()
{
    pitch = 0;
    yaw = 0;

    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 10;
}

void fmri::configureCamera()
{
    glLoadIdentity();
    glRotatef(yaw, 0, 1, 0);
    glRotatef(pitch, 1, 0, 0);
    glTranslatef(-pos[0], -pos[1], -pos[2]);
}
