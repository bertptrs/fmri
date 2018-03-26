#include <algorithm>
#include "Texture.hpp"

using namespace fmri;

Texture::Texture() noexcept
{
    glGenTextures(1, &id);
}

Texture::Texture(GLuint id) noexcept : id(id)
{
}

Texture::~Texture()
{
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

Texture &Texture::operator=(Texture && other) noexcept
{
    std::swap(id, other.id);
    return *this;
}

Texture::Texture(Texture && other) noexcept
{
    std::swap(id, other.id);
}

void Texture::bind(GLenum target) const
{
    glBindTexture(target, id);
}

void Texture::configure(GLenum target) const
{
    bind(target);
    const float color[] = {1, 0, 1}; // Background color for textures.

    // Set up (lack of) repetition
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

    // Set up texture scaling
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // Use mipmapping for scaling down
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest pixel when scaling up.
}
