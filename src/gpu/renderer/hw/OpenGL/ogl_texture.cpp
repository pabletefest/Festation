#include "ogl_texture.hpp"

#include <glad/gl.h>

#include <print>
#include <cassert>

struct GLTextureFormat {
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

static constexpr auto getGLFormat(festation::TextureFormat format) -> GLTextureFormat {
    switch(format) {
        case festation::TextureFormat::RGB5:
            return { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 };
        case festation::TextureFormat::RGB8:
            return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case festation::TextureFormat::RGBA8:
            return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
        default:
            std::unreachable();
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

auto festation::OGLTexture::setData(const uint8_t *const buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void
{
    updateTexture(buffer, offset, size);

    if (m_specification.useMipmaps) {
        generateMipmap();
    }
}

auto festation::OGLTexture::setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void
{
    updateTexture(buffer.data(), offset, size);

    if (m_specification.useMipmaps) {
        generateMipmap();
    }
}

auto festation::OGLTexture::createTexture() -> void
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    GLenum filtering;

    switch (m_specification.filtering)
    {
    case TextureFiltering::NEAREST:
        filtering = GL_NEAREST;
        break;
    case TextureFiltering::LINEAR:
        filtering = GL_LINEAR;
        break;
    default:
        std::unreachable();
    }
  
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, filtering);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, filtering);

    auto [internalFormat, _1, _2] = getGLFormat(m_specification.format);
    
    glTextureStorage2D(m_textureID, 1, internalFormat, m_specification.size.x, m_specification.size.y);
}

auto festation::OGLTexture::updateTexture(const uint8_t *const data, const glm::uvec2& offset, const glm::uvec2& size) -> void
{
    auto [_, format, type] = getGLFormat(m_specification.format);
    glTextureSubImage2D(m_textureID, 0, offset.x, offset.y, size.x, size.y, format, type, data);
}

auto festation::OGLTexture::generateMipmap() -> void
{
    glGenerateTextureMipmap(m_textureID);
}