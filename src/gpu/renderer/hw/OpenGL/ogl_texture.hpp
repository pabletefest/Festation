#pragma once

#include "gpu/renderer/texture.hpp"

namespace festation {
    class OGLTexture : public ITexture {
        public:
            OGLTexture(const TextureInfo& specification);
            ~OGLTexture() override;

            auto apply(uint32_t textureSlot) -> void override;

            auto setData(const uint8_t *const buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void override;
            auto setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void override;

        private:
            auto createTexture() -> void;
            auto updateTexture(const uint8_t *const data, const glm::uvec2& offset, const glm::uvec2& size) -> void;
            auto generateMipmap() -> void;
    };
};