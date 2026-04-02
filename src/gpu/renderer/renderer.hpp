#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "gpu/primitives_data.hpp"
#include "framebuffer.hpp"

#include <memory>
#include <vector>
#include <span>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <glad/gl.h>

namespace festation {

    enum TexturePageColorsDepth : uint32_t {
        Color4bit = 0,
        Color8bit = 1,
        Color15bit = 2,
        ColorReserved = 3,
    };

    class Renderer {
    public:
        Renderer(const std::vector<uint16_t>& vram);
        ~Renderer();

        auto setClearColor(const glm::vec4& color) -> void;
        auto clearDisplay() -> void;

        auto setViewport(const glm::uvec2& startCoord, const glm::uvec2& size) -> void;
        auto setClipRegion(const glm::uvec2& startCoord, const glm::uvec2& size) -> void;
        auto setProjection(const glm::mat4& projection) -> void;
        auto enableDepthTesting() -> void;
        auto disableDepthTesting() -> void;
        auto enableBlending() -> void;
        auto disableBlending() -> void;

        auto uploadVramToGpu(const uint8_t* data, 
            const glm::uvec2& offset, const glm::uvec2& size) -> void;
        auto uploadVramToGpu(std::span<uint8_t> data, 
            const glm::uvec2& offset, const glm::uvec2& size) -> void;

        auto drawRectangle(const RectanglePrimitiveData& rectData) -> void;
        auto drawRectangleTextured(const RectanglePrimitiveData& rectData, TexturePageColorsDepth colorDepth, 
            const std::vector<uint16_t>& vram) -> void;

        auto drawPolygon(const PolygonPrimitiveData& polygonData) -> void;
        auto drawPolygonTextured(const PolygonPrimitiveData& polygonData, TexturePageColorsDepth colorDepth, 
            const std::vector<uint16_t>& vram) -> void;

        auto renderBatch() -> void;

    private:
        std::unique_ptr<IShader> m_flatColorShader{};
        std::unique_ptr<IShader> m_textureShader{};
        GLuint m_VAO{}, m_VBO{}, m_IBO{};
        std::vector<PrimitiveVertex> m_vertices{};
        std::vector<GLuint> m_indices{};
        size_t m_indicesCount{};
        glm::mat4 m_projection{};
        std::unique_ptr<IFramebuffer> m_vramFramebuffer{};
        std::unique_ptr<ITexture> m_defaultWhiteTexture{};
        std::unique_ptr<ITexture> m_vramRawTexture{};
        const std::vector<uint16_t>& m_vramRef;

        inline static std::filesystem::path SHADERS_PATH { std::filesystem::current_path().concat("/../../../res/shaders/") };
    };
};
