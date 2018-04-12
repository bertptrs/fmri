#pragma once

#include <memory>
#include <GL/gl.h>

namespace fmri
{
    /**
     * Simple owning Texture class.
     *
     * Encapsulates an OpenGL texture, and enables RAII for it. Copying
     * is disallowed for this reason.
     */
    class Texture
    {
    public:
        Texture() noexcept;
        Texture(const float* data, int width, int height, GLuint format, int subImages = 1);
        Texture(std::unique_ptr<float[]> &&data, int width, int height, GLuint format, int subImages = 1);
        Texture(Texture &&) noexcept;
        Texture(const Texture &) = delete;

        ~Texture();

        Texture &operator=(Texture &&) noexcept;
        Texture &operator=(const Texture &) = delete;

        /**
         * Bind the owned texture to the given spot.
         * @param target valid target for glBindTexture.
         */
        void bind(GLenum target) const;
        void configure(GLenum target);

    private:
        GLuint id;
        int width;
        int height;
        GLuint format;
        std::unique_ptr<float[]> data;

        void ensureReference();

        void preCalc(int subImages);

        int getStride();
    };
}
