#include "renderer.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

#include <filesystem>

#include <glad/gl.h>

static std::filesystem::path SHADERS_PATH = "../../../shaders/";

festation::Renderer::Renderer() 
    : m_shader(std::make_unique<OGLShader>(SHADERS_PATH.concat("flat_color.glsl.vert"),
        SHADERS_PATH.concat("flat_color.glsl.frag")))
{
    setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearDisplay();
}

festation::Renderer::~Renderer()
{
}

void festation::Renderer::setClearColor(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void festation::Renderer::clearDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);
}
