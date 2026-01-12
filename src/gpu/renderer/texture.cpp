#include "texture.hpp"
#include "hw/OpenGL/ogl_texture.hpp"

auto festation::ITexture::createUnique(const TextureInfo &specification) -> std::unique_ptr<ITexture>
{
    return std::make_unique<festation::OGLTexture>(specification);
}

auto festation::ITexture::createShared(const TextureInfo &specification) -> std::shared_ptr<ITexture>
{
    return std::make_shared<festation::OGLTexture>(specification);
}

festation::ITexture::ITexture(const TextureInfo &specification)
    : m_specification(specification)
{
}