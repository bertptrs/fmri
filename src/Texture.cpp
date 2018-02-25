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
