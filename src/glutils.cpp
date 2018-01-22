#include <cstdint>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <cstring>
#include <glog/logging.h>
#include "glutils.hpp"

using namespace fmri;
using namespace std;

GLuint fmri::loadTexture(DType const *data, int width, int height)
{
    // Load and scale texture
    vector<DType> textureBuffer(data, data + (width * height));
    rescale(textureBuffer.begin(), textureBuffer.end(), 0, 1);

    const float color[] = {0, 0, 0}; // Background color for textures.
    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    // Set up (lack of) repetition
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    static_assert(sizeof(DType) == 4); // verify data type for texture.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 2, 2, 0, GL_R32F, GL_FLOAT, textureBuffer.data());

    // Set up texture scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmapping for scaling down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest pixel when scaling up.

    glGenerateMipmap(GL_TEXTURE_2D);


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
        switch (glGetError()) {
            case GL_NO_ERROR:
                // Should not get here, but whatever.
                return;

            default:
                cerr << "OpenGL error: " << (const char*) glewGetErrorString(error) << endl;
        }
    }
}
