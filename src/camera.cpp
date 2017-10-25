#include <GL/glut.h>
#include "camera.hpp"

static float yaw, pitch;

static struct {
    float x, y, z;
} camera;

static void handleMouseMove(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    yaw = (x - width) / width * 180;
    pitch = (y - height) / height * 90;
}

static void handleKeys(unsigned char key, int, int)
{
    constexpr float rotationScaling = 0.2f;
    constexpr float movementScaling = 0.2f;
    switch (key) {
        case 'a':
            // TODO: handle rotations
            camera.x += movementScaling;
            break;

        case 'd':
            camera.x -= rotationScaling;
            break;

        case 'w':
            camera.z += movementScaling;
            break;

        case 's':
            camera.z -= movementScaling;
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
    camera.x = 0;
    camera.y = 0;
    camera.z = -10;

    pitch = 0;
    yaw = 0;
}

void fmri::configureCamera()
{
    glLoadIdentity();
    glRotatef(yaw, 0, 1, 0);
    glRotatef(pitch, 1, 0, 0);
    glTranslatef(camera.x, camera.y, camera.z);
}
