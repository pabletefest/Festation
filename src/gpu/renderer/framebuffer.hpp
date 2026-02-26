#pragma once

#include "gpu/renderer/texture.hpp"

#include <span>
#include <cstdint>
#include <memory>

#include <glm/vec2.hpp>

namespace festation {
    enum class FboFormats {
        RGBA8,
        RGBA5,
    };

    struct FramebufferInfo {
        glm::uvec2 size;
        FboFormats format;
    };

    class IFramebuffer {
    public:
        virtual ~IFramebuffer() = default;

        virtual auto apply() -> void = 0;

        virtual auto setData(const uint8_t* buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void = 0;
        virtual auto setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void = 0;

        virtual auto blitToSwapchain() -> void = 0;
        virtual auto blitToFramebuffer(const IFramebuffer& framebuffer, const glm::uvec2& srcOffset, const glm::uvec2& dstOffset) -> void = 0;

        auto getHandle() const -> uint32_t { return m_fbo; }
        auto getFramebufferInfo() const -> FramebufferInfo { return m_specification; }

        static auto createUnique(const FramebufferInfo& specification) -> std::unique_ptr<IFramebuffer>;
        static auto createShared(const FramebufferInfo& specification) -> std::shared_ptr<IFramebuffer>;

    protected:
        IFramebuffer(const FramebufferInfo& specification);

    protected:
        uint32_t m_fbo;
        FramebufferInfo m_specification;
        std::unique_ptr<ITexture> m_colorAttachment;
    };
};