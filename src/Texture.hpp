#pragma once

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
        /**
         * Allocate a new texture
         */
        Texture() noexcept;
        Texture(Texture &&) noexcept;
        Texture(const Texture &) = delete;

        /**
         * Own an existing texture
         * @param id original texture ID.
         */
        explicit Texture(GLuint id) noexcept;

        ~Texture();

        Texture &operator=(Texture &&) noexcept;
        Texture &operator=(const Texture &) = delete;

        /**
         * Bind the owned texture to the given spot.
         * @param target valid target for glBindTexture.
         */
        void bind(GLenum target) const;

    private:
        GLuint id;

    };
}
