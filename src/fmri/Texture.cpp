#include <algorithm>
#include <glog/logging.h>
#include <GL/glu.h>
#include "Texture.hpp"
#include "utils.hpp"

using namespace fmri;

Texture::~Texture()
{
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

Texture &Texture::operator=(Texture && other) noexcept
{
    std::swap(id, other.id);
    std::swap(width, other.width);
    std::swap(height, other.height);
    std::swap(data, other.data);
    std::swap(format, other.format);
    return *this;
}

void Texture::bind(GLenum target) const
{
    CHECK_NE(id, 0) << "Texture doesn't hold a reference!";
    glBindTexture(target, id);
}

void Texture::configure(GLenum target)
{
    ensureReference();
    CHECK(data) << "No valid data to configure with";
    bind(target);
    const float color[] = {1, 0, 1}; // Background color for textures.

    // Set up (lack of) repetition
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

    // Set up texture scaling
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // Use mipmapping for scaling down
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest pixel when scaling up.
    gluBuild2DMipmaps(target, format, width, height, format, GL_FLOAT, data.get());
    data.reset(); // Release data buffer.
}

void Texture::ensureReference()
{
    if (id == 0) {
        glGenTextures(1, &id);
        CHECK_NE(id, 0) << "Failed to allocate a texture.";
    }
}

Texture::Texture(const float *data, int width, int height, GLuint format, int subImages) :
        Texture(std::make_unique<float[]>(width * height), width, height, format, subImages)
{
    std::copy_n(data, width * height, this->data.get());
    preCalc(subImages);
}

Texture::Texture(std::unique_ptr<float[]> &&data, int width, int height, GLuint format, int subImages) :
    id(0),
    width(width),
    height(height),
    format(format),
    data(std::move(data))
{
    preCalc(subImages);
}

void Texture::preCalc(int subImages)
{
    CHECK_EQ(height % subImages, 0) << "Image should be properly divisible!";

    // Rescale images
    const auto step = width * height * getStride() / subImages;
    auto cur = data.get();
    for (auto i = 0; i < subImages; ++i) {
        rescale(cur, cur + step, 0, 1);
        std::advance(cur, step);
    }
}

int Texture::getStride()
{
    switch (format) {
        case GL_RGB:
            return 3;

        case GL_LUMINANCE:
            return 1;

        default:
            LOG(ERROR) << "Stride unknown for format: " << format;
            exit(1);
    }
}

Texture::Texture() noexcept :
id(0)
{
}
