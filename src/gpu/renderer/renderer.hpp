#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "gpu/primitives_data.hpp"

#include <memory>
#include <vector>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <glad/gl.h>

namespace festation {

    struct PrimitiveVertex {
        glm::vec2 coords;
        glm::vec4 color; 
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void setClearColor(const glm::vec4& color);
        void clearDisplay();

        void setViewport(const glm::ivec2& startCoord, const glm::ivec2& size);
        void setClipRegion(const glm::ivec2& startCoord, const glm::ivec2& size);
        void setProjection(const glm::mat4& projection);

        void drawRectangle(const RectanglePrimitiveData& rectData);

        void renderFrame();

    private:
        std::unique_ptr<IShader> m_flatColorShader{};
        GLuint m_VAO, m_VBO, m_IBO;
        std::vector<PrimitiveVertex> m_vertices;
        std::vector<GLuint> m_indices;
        size_t m_indicesCount;
        glm::mat4 m_projection;

        inline static std::filesystem::path SHADERS_PATH { std::filesystem::current_path().concat("/../../../res/shaders/") };
    };
};
