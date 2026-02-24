#pragma once

#include "gpu/renderer/framebuffer.hpp"

namespace festation {
    class OGLFramebuffer : public Framebuffer {
    public:
        OGLFramebuffer(const FramebufferInfo& specification);
        ~OGLFramebuffer();

        auto apply() -> void override;

        auto setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void override;

        auto blitToSwapchain() -> void override;
        auto blitToFramebuffer(const Framebuffer& framebuffer, const glm::uvec2& srcOffset, const glm::uvec2& dstOffset) -> void override;
    };
};
