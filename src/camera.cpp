#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include "camera.hpp"

using namespace fmri;
using namespace std;

static Camera& camera = Camera::instance();

static void handleMouseMove(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    camera.angle[0] = (x - width) / width * 180;
    camera.angle[1] = (y - height) / height * 90;
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
        camera.pos[i] += speed * dir[i];
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

std::string Camera::infoLine()
{
    stringstream buffer;
    buffer << "Pos(x,y,z) = (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
    buffer << "Angle(p,y) = (" << angle[0] << ", " << angle[1] << ")\n";
    return buffer.str();
}

void Camera::reset()
{
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 10;

    angle[0] = 0;
    angle[1] = 0;
}

void Camera::configureRenderingContext()
{
    glLoadIdentity();
    glRotatef(angle[0], 0, 1, 0);
    glRotatef(angle[1], 1, 0, 0);
    glTranslatef(-pos[0], -pos[1], -pos[2]);
}

Camera &Camera::instance()
{
    static Camera camera;
    return camera;
}

void Camera::registerControls()
{
    reset();
    glutPassiveMotionFunc(handleMouseMove);
    glutKeyboardFunc(handleKeys);
}
