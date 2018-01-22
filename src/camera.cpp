#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include "camera.hpp"
#include "utils.hpp"

using namespace fmri;
using namespace std;

static Camera& camera = Camera::instance();

static void handleMouseMove(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    camera.angle[0] = (x - width) / width * 180;
    camera.angle[1] = (y - height) / height * 90;

    glutPostRedisplay();
}

static void move(unsigned char key)
{
    float speed = 0.5;
    float dir[3];

    const auto yaw = deg2rad(camera.angle[0]);
    const auto pitch = deg2rad(camera.angle[1]);

    if (key == 'w' || key == 's') {
        dir[0] = sin(yaw) * cos(pitch);
        dir[1] = -sin(pitch);
        dir[2] = -cos(yaw) * cos(pitch);
    } else {
        dir[0] = -cos(yaw);
        dir[1] = 0;
        dir[2] = -sin(yaw);
    }

    if (key == 's' || key == 'd') {
        speed *= -1;
    }

    for (auto i = 0; i < 3; ++i) {
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

        case 'h':
            camera.reset();
            break;

        default:
            // Do nothing.
            break;
    }

    glutPostRedisplay();
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
    pos[2] = 3;

    angle[0] = 0;
    angle[1] = 0;
}

void Camera::configureRenderingContext()
{
    glLoadIdentity();
    glRotatef(angle[1], 1, 0, 0);
    glRotatef(angle[0], 0, 1, 0);
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
