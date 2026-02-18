#pragma once

#include <span>
#include <cstdint>
#include <filesystem>
#include <memory>

namespace festation {
    enum class TextureFormat {
        NONE,
        RGB8,
        RGBA8
    };

    struct TextureInfo {
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        bool useMipmaps;
    };

    class ITexture {
        public:
            virtual ~ITexture() = default;

            virtual auto apply(uint32_t textureSlot) -> void = 0;

            virtual auto setData(const uint8_t *const buffer) -> void = 0;
            virtual auto setData(std::span<uint8_t> buffer) -> void = 0;

            static auto createUnique(const TextureInfo& specification) -> std::unique_ptr<ITexture>;
            static auto createShared(const TextureInfo& specification) -> std::shared_ptr<ITexture>;

        protected:
            ITexture(const TextureInfo& specification);

        protected:
            uint32_t m_textureID;
            TextureInfo m_specification;
    };
};