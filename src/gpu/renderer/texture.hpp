#pragma once

#include <span>
#include <cstdint>
#include <filesystem>
#include <memory>

#include <glm/vec2.hpp>

namespace festation {
    enum class TextureFormat {
        NONE,
        RGB8,
        RGBA8,
        RGB5,
    };

    enum class TextureFiltering {
        NEAREST,
        LINEAR,
    };

    struct TextureInfo {
        glm::uvec2 size;
        TextureFormat format;
        TextureFiltering filtering;
        bool useMipmaps;
    };

    class ITexture {
        public:
            virtual ~ITexture() = default;

            virtual auto apply(uint32_t textureSlot) -> void = 0;

            virtual auto setData(const uint8_t *const buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void = 0;
            virtual auto setData(std::span<uint8_t> buffer, const glm::uvec2& offset, const glm::uvec2& size) -> void = 0;

            auto getHandle() const -> uint32_t { return m_textureID; }
            auto getTextureInfo() const -> TextureInfo { return m_specification; }

            static auto createUnique(const TextureInfo& specification) -> std::unique_ptr<ITexture>;
            static auto createShared(const TextureInfo& specification) -> std::shared_ptr<ITexture>;

        protected:
            ITexture(const TextureInfo& specification);

        protected:
            uint32_t m_textureID;
            TextureInfo m_specification;
    };
};