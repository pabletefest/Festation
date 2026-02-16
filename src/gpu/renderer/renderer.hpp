#pragma once

#include "shader.hpp"
#include "texture.hpp"

#include <memory>

#include <glm/vec4.hpp>

namespace festation {
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void setClearColor(const glm::vec4& color);
        void clearDisplay();

    private:
        std::unique_ptr<IShader> m_shader;
    };
};
