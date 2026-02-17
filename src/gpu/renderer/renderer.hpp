#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "gpu/primitives_data.hpp"

#include <memory>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace festation {
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void setClearColor(const glm::vec4& color);
        void clearDisplay();

        void setViewport(const glm::ivec2& startCoord, const glm::ivec2& size);
        void setClipRegion(const glm::ivec2& startCoord, const glm::ivec2& size);

        void drawRectangle(const RectanglePrimitiveData& rectData);

    private:
        std::unique_ptr<IShader> m_shader{};

        inline static std::filesystem::path SHADERS_PATH { std::filesystem::current_path().concat("/../../../res/shaders/") };
    };
};
