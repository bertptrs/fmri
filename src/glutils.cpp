#include <cstdint>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <cstring>
#include <glog/logging.h>
#include <chrono>
#include <thread>
#include "glutils.hpp"
#include "Range.hpp"
#include "Texture.hpp"

using namespace fmri;
using namespace std;

static void handleGLError(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return;

        default:
            cerr << "OpenGL error: " << (const char*) glewGetErrorString(error) << endl;
    }
}

static void rescaleSubImages(vector<float>& textureBuffer, int subImages) {
    auto cur = textureBuffer.begin();
    const auto increment = textureBuffer.size() / subImages;

    while (cur != textureBuffer.end()) {
        rescale(cur, cur + increment, 0, 1);
        advance(cur, increment);
    }
}

fmri::Texture fmri::loadTexture(DType const *data, int width, int height, int subImages)
{
    // Load and scale texture
    vector<float> textureBuffer(data, data + (width * height));
    rescaleSubImages(textureBuffer, subImages);

    const float color[] = {1, 1, 1}; // Background color for textures.

    Texture texture;
    texture.bind(GL_TEXTURE_2D);

    // Set up (lack of) repetition
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    // Set up texture scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // Use mipmapping for scaling down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest pixel when scaling up.
    checkGLErrors();

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, width, height, GL_LUMINANCE, GL_FLOAT, textureBuffer.data());

    return texture;
}

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

void fmri::renderText(std::string_view text)
{
    constexpr auto font = GLUT_BITMAP_HELVETICA_10;
    glRasterPos2i(0, 0);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void fmri::checkGLErrors()
{
    while (auto error = glGetError()) {
        handleGLError(error);
    }
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
