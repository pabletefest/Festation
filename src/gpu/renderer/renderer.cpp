#include "renderer.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

#include <filesystem>
#include <array>

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

void festation::Renderer::drawRectangle(const RectanglePrimitiveData &rectData, const glm::mat4& projection)
{
    std::array<glm::vec2, 4> vertices = {
        glm::vec2(rectData.vertex1.x, rectData.vertex1.y),
        glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y),
        glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y + rectData.size.y),
        glm::vec2(rectData.vertex1.x, rectData.vertex1.y + rectData.size.y),
    };

    std::array<GLuint, 6> indices = {
        0, 1, 2, 2, 3, 0,
    };

    GLuint m_VAO, m_VBO, m_IBO;

    glCreateVertexArrays(1, &m_VAO);

    glCreateBuffers(1, &m_VBO);
    glNamedBufferStorage(m_VBO, sizeof(glm::vec2) * vertices.size(), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &m_IBO);
    glNamedBufferStorage(m_IBO, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
    
    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(glm::vec2));
    glVertexArrayElementBuffer(m_VAO, m_IBO);

    glEnableVertexArrayAttrib(m_VAO, 0);
    glVertexArrayAttribFormat(m_VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_VAO, 0, 0);

    m_shader->apply();
    m_shader->setData("uProjection", projection);

    glm::vec4 color = {
        rectData.color.r / 255.0f,
        rectData.color.g / 255.0f,
        rectData.color.b / 255.0f,
        rectData.color.a,
    };

    m_shader->setData("uColor", color);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_IBO);
    glDeleteVertexArrays(1, &m_VAO);
}
