#include "ogl_framebuffer.hpp"
#include "utils/logger.hpp"

#include <glad/gl.h>

festation::OGLFramebuffer::OGLFramebuffer(const FramebufferInfo &specification)
    : Framebuffer(specification)
{
    glCreateFramebuffers(1,&m_fbo);
    glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_colorAttachment->getHandle(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERROR("Framebuffer is not complete!");
	}
}

festation::OGLFramebuffer::~OGLFramebuffer()
{
    glDeleteFramebuffers(1, &m_fbo);
}

auto festation::OGLFramebuffer::apply() -> void
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

auto festation::OGLFramebuffer::setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void
{
    if (size.x > m_specification.size.x || size.y > m_specification.size.y) {
        TextureInfo textInfo = m_colorAttachment->getTextureInfo();
        textInfo.size = size;
        m_colorAttachment = ITexture::createUnique(textInfo);
    }

    m_colorAttachment->setData(buffer, offset, size);
}

auto festation::OGLFramebuffer::blitToSwapchain() -> void
{
	glBlitNamedFramebuffer(m_fbo, 0, 
        0, 0, m_specification.size.x, m_specification.size.y, // Source rect
		0, 0, m_specification.size.x, m_specification.size.y, // Destination rect
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

auto festation::OGLFramebuffer::blitToFramebuffer(const Framebuffer &framebuffer, const glm::uvec2& srcOffset, const glm::uvec2& dstOffset) -> void
{
	glBlitNamedFramebuffer(m_fbo, framebuffer.getHandle(), 
        srcOffset.x, srcOffset.y, m_specification.size.x + srcOffset.x, m_specification.size.y + srcOffset.y,                                   // Source rect
		dstOffset.x, dstOffset.y, framebuffer.getFramebufferInfo().size.x + dstOffset.x, framebuffer.getFramebufferInfo().size.y + dstOffset.y, // Destination rect
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
