#pragma once

#include "LayerData.hpp"
#include "utils.hpp"
#include <GL/glut.h>

namespace fmri {
    /**
     * Create a texture from an array of data.
     *
     * @param data
     * @param width
     * @param height
     * @return A texture reference.
     */
    GLuint loadTexture(DType const * data, int width, int height);

    /**
     *
     * @param type
     * @param source
     * @return
     */
    GLuint compileShader(GLenum type, char const * source);

    /**
     * Callback handler to handle resizing windows.
     *
     * This function resizes the rendering viewport so everything still
     * looks proportional.
     *
     * @param w new Width
     * @param h new Height.
     */
    void changeWindowSize(int w, int h);

    /**
     * Set the current drawing color based on some intensity value.
     *
     * @param i The intensity.
     */
    void setColorFromIntensity(float i);
}
