#include "renderer.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

#include <filesystem>

#include <glad/gl.h>

festation::Renderer::Renderer() 
    : m_shader(IShader::createUnique(SHADERS_PATH / "flat_color.glsl.vert",
        SHADERS_PATH / "flat_color.glsl.frag"))
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

void festation::Renderer::setViewport(const glm::ivec2 &startCoord, const glm::ivec2 &size)
{
    glViewport(startCoord.x, startCoord.y, size.x, size.y);
}

void festation::Renderer::setClipRegion(const glm::ivec2 &startCoord, const glm::ivec2 &size)
{
    glScissor(startCoord.x, startCoord.y, size.x, size.y);
}

void festation::Renderer::drawRectangle(const RectanglePrimitiveData &rectData)
{
}
