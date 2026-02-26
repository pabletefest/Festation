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
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        auto setClearColor(const glm::vec4& color) -> void;
        auto clearDisplay() -> void;

        auto setViewport(const glm::ivec2& startCoord, const glm::ivec2& size) -> void;
        auto setClipRegion(const glm::ivec2& startCoord, const glm::ivec2& size) -> void;
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

        auto renderFrame() -> void;

    private:
        std::unique_ptr<IShader> m_flatColorShader{};
        GLuint m_VAO{}, m_VBO{}, m_IBO{};
        std::vector<PrimitiveVertex> m_vertices{};
        std::vector<GLuint> m_indices{};
        size_t m_indicesCount{};
        glm::mat4 m_projection{};
        std::unique_ptr<IFramebuffer> m_vramFramebuffer{};

        inline static std::filesystem::path SHADERS_PATH { std::filesystem::current_path().concat("/../../../res/shaders/") };
    };
};
