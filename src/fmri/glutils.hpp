#pragma once

#include "LayerData.hpp"
#include "utils.hpp"
#include "Texture.hpp"
#include <GL/glut.h>
#include <string_view>

namespace fmri {
    /**
     * Create a texture from an array of data.
     *
     * @param data
     * @param width
     * @param height
     * @param subImages Number of subimages in the original image. Sub images are rescaled individually to preserve contrast. Optional, default 1.
     * @return A texture reference.
     */
    fmri::Texture loadTexture(DType const *data, int width, int height, int subImages = 1);

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
     * Draw a bitmap string at the current location.
     *
     * This function wraps glutBitmapCharacter, because its nicer version
     * (glutBitmapString) is an extension that is possibly unavailable.
     *
     * @param text The text to draw.
     */
    void renderText(std::string_view text, int x = 0, int y = 0);

    /**
     * Check if there are OpenGL errors and report them.
     */
    void checkGLErrors();

    /**
     * Slow down until the idle func is being called a reasonable amount of times.
     */
    void throttleIdleFunc();

    void setOrthographicProjection();

    void restorePerspectiveProjection();

    /**
     * Draw a series of textured tiles to the screen.
     *
     * This function ends up drawing GL_QUADS.
     *
     * @param n Number of vertices
     * @param vertexBuffer
     * @param textureCoords
     * @param texture
     */
    void drawImageTiles(int n, const float* vertexBuffer, const float* textureCoords, const Texture& texture);
}
