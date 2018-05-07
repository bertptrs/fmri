#include <cstdint>
#include <memory>
#include <vector>
#include <cstring>
#include <glog/logging.h>
#include <chrono>
#include <thread>
#include "glutils.hpp"

#ifdef FREEGLUT
#include <GL/freeglut.h>
#endif

using namespace fmri;
using namespace std;

void fmri::changeWindowSize(int w, int h)
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;

    float ratio = w * 1.0f / h;

    // Use the Projection Matrix
    glMatrixMode(GL_PROJECTION);

    // Reset Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective.
    gluPerspective(45.0f, ratio, 0.1f, 10000.0f);

    // Get Back to the Modelview
    glMatrixMode(GL_MODELVIEW);
}

void fmri::renderText(std::string_view text, int x, int y)
{
    constexpr auto font = GLUT_BITMAP_HELVETICA_10;
    glRasterPos2i(x, y);
#ifdef FREEGLUT
    std::string textBuffer(text);
    glutBitmapString(font, reinterpret_cast<const unsigned char *>(textBuffer.c_str()));
#else
    for (char c : text) {
        if (c == '\n') {
            y += 12;
            glRasterPos2i(x, y);
        } else {
            glutBitmapCharacter(font, c);
        }
    }
#endif
}

void fmri::throttleIdleFunc()
{
    using namespace std::chrono;

    constexpr duration<double, ratio<1, 60>> refreshRate(1);

    static auto lastCalled = steady_clock::now();

    const auto now = steady_clock::now();

    const auto diff = now - lastCalled;

    if (diff < refreshRate) {
        this_thread::sleep_for(refreshRate - diff);
    }

    lastCalled = now;
}

void fmri::restorePerspectiveProjection() {

    glMatrixMode(GL_PROJECTION);
    // restore previous projection matrix
    glPopMatrix();

    // get back to modelview mode
    glMatrixMode(GL_MODELVIEW);
}

void fmri::setOrthographicProjection() {

    // switch to projection mode
    glMatrixMode(GL_PROJECTION);

    // save previous matrix which contains the
    //settings for the perspective projection
    glPushMatrix();

    // reset matrix
    glLoadIdentity();

    // set a 2D orthographic projection
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0);

    // switch back to modelview mode
    glMatrixMode(GL_MODELVIEW);
}

void fmri::drawImageTiles(int n, const float *vertexBuffer, const float *textureCoords, const Texture &texture, float alpha)
{
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, alpha);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    texture.bind(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_FLOAT, 0, textureCoords);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
    glDrawArrays(GL_QUADS, 0, n);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void fmri::registerErrorCallbacks()
{
#ifdef FREEGLUT
    glutInitErrorFunc([](const char* format, va_list args) {
        char buffer[1024];
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        LOG(ERROR) << "freeglut: " << buffer;
    });
    glutInitWarningFunc([](const char* format, va_list args) {
        char buffer[1024];
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        LOG(WARNING) << "freeglut: " << buffer;
    });
    LOG(INFO) << "Freeglut error handlers installed successfully";
#else
    LOG(INFO) << "Compiled without freeglut, error handlers not available.";
#endif
}

void fmri::setGlColor(const Color &c)
{
    if constexpr (alphaEnabled()) {
        glColor4fv(c.data());
    } else {
        glColor3fv(c.data());
    }
}
