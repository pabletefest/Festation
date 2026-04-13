#include "framebuffer.hpp"
#include "hw/OpenGL/ogl_framebuffer.hpp"

#include <utility>

auto festation::IFramebuffer::createUnique(const FramebufferInfo &specification) -> std::unique_ptr<IFramebuffer>
{
    return std::make_unique<OGLFramebuffer>(specification);
}

auto festation::IFramebuffer::createShared(const FramebufferInfo &specification) -> std::shared_ptr<IFramebuffer>
{
    return std::make_shared<OGLFramebuffer>(specification);
}

festation::IFramebuffer::IFramebuffer(const FramebufferInfo &specification)
    : m_specification(specification)
{
    TextureInfo textInfo = {
        .size = m_specification.size,
        .filtering = TextureFiltering::LINEAR,
        .useMipmaps = true,
    };

    switch (specification.format) {
    case FboFormats::RGBA5:
        textInfo.format = TextureFormat::RGB5;
        break;
    case FboFormats::RGBA5_REV:
        textInfo.format = TextureFormat::RGB5_REV;
        break;
    case FboFormats::RGBA8:
        textInfo.format = TextureFormat::RGBA8;
        break;
    case FboFormats::R16UI:
        textInfo.format = TextureFormat::R16UI;
        break;
    default:
        std::unreachable();
    }

    m_colorAttachment = ITexture::createUnique(textInfo);
}
