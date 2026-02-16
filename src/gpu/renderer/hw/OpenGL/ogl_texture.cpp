#include "ogl_texture.hpp"

#include <glad/gl.h>

#include <print>
#include <cassert>

struct GLTextureFormat {
    GLenum internalFormat;
    GLenum format;
};

static constexpr auto getGLFormat(festation::TextureFormat format) -> GLTextureFormat {
    switch(format) {
        case festation::TextureFormat::RGB8:
            return { GL_RGB8, GL_RGB };
        case festation::TextureFormat::RGBA8:
            return { GL_RGBA8, GL_RGBA };
        default:
            assert(false && "Invalid texture format!");
    }

    return {};
}

festation::OGLTexture::OGLTexture(const festation::TextureInfo &specification)
    : ITexture(specification)
{
    createTexture();
}

festation::OGLTexture::~OGLTexture()
{
    glBindTextureUnit(0, 0);
    glDeleteTextures(1, &m_textureID);
}

auto festation::OGLTexture::apply(uint32_t textureSlot) -> void
{
    glBindTextureUnit(textureSlot, m_textureID);
}

auto festation::OGLTexture::setData(const uint8_t *const buffer) -> void
{
    updateTexture(buffer);

    if (m_specification.useMipmaps) {
        generateMipmap();
    }
}

auto festation::OGLTexture::setData(std::span<uint8_t> buffer) -> void
{
    updateTexture(buffer.data());

    if (m_specification.useMipmaps) {
        generateMipmap();
    }
}

auto festation::OGLTexture::createTexture() -> void
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
  
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto [internalFormat, _] = getGLFormat(m_specification.format);
    
    glTextureStorage2D(m_textureID, 1, internalFormat, m_specification.width, m_specification.height);
}

auto festation::OGLTexture::updateTexture(const uint8_t *const data) -> void
{
    auto [_, format] = getGLFormat(m_specification.format);
    glTextureSubImage2D(m_textureID, 0, 0, 0, m_specification.width, m_specification.height, format, GL_UNSIGNED_BYTE, data);
}

auto festation::OGLTexture::generateMipmap() -> void
{
    glGenerateTextureMipmap(m_textureID);
}