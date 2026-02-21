#include "renderer.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

#include <filesystem>
#include <array>
#include <cstddef>

static constexpr size_t MAX_VERTICES_PER_PRIMITIVE = 4;
static constexpr size_t MAX_INDICES_PER_PRIMITIVE = 6;
static constexpr size_t MAX_PRIMITIVES_COUNT = 1024 * 512;
static constexpr size_t MAX_VERTICES_COUNT = MAX_PRIMITIVES_COUNT * MAX_VERTICES_PER_PRIMITIVE;
static constexpr size_t MAX_INDICES_COUNT = MAX_PRIMITIVES_COUNT * MAX_INDICES_PER_PRIMITIVE;

festation::Renderer::Renderer()
    : m_flatColorShader(IShader::createUnique(SHADERS_PATH / "flat_color.glsl.vert",
        SHADERS_PATH / "flat_color.glsl.frag")), m_VAO(0), m_VBO(0), m_IBO(0), m_projection(glm::mat4(1)),
            m_indicesCount(0)
{
    m_indices.resize(MAX_INDICES_COUNT);
    size_t offset = 0;

    for (size_t i = 0; i < MAX_INDICES_COUNT; i += 6) {
        m_indices[i + 0] = 0 + offset;
        m_indices[i + 1] = 1 + offset;
        m_indices[i + 2] = 2 + offset;
        m_indices[i + 3] = 2 + offset;
        m_indices[i + 4] = 3 + offset;
        m_indices[i + 5] = 0 + offset;

        offset += 4;
    }

    glCreateVertexArrays(1, &m_VAO);

    glCreateBuffers(1, &m_VBO);
    glNamedBufferStorage(m_VBO, sizeof(PrimitiveVertex) * MAX_VERTICES_PER_PRIMITIVE * MAX_PRIMITIVES_COUNT, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &m_IBO);
    glNamedBufferStorage(m_IBO, sizeof(GLuint) * m_indices.size(), m_indices.data(), GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(PrimitiveVertex));
    glVertexArrayElementBuffer(m_VAO, m_IBO);

    glEnableVertexArrayAttrib(m_VAO, 0);
    glEnableVertexArrayAttrib(m_VAO, 1);

    glVertexArrayAttribFormat(m_VAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(PrimitiveVertex, coords));
    glVertexArrayAttribFormat(m_VAO, 1, 4, GL_FLOAT, GL_FALSE, offsetof(PrimitiveVertex, color));

    glVertexArrayAttribBinding(m_VAO, 0, 0);
    glVertexArrayAttribBinding(m_VAO, 1, 0);

    glEnable(GL_SCISSOR_TEST);

    setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearDisplay();
}

festation::Renderer::~Renderer()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_IBO);
    glDeleteVertexArrays(1, &m_VAO);
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

void festation::Renderer::setProjection(const glm::mat4 &projection)
{
    m_projection = projection;
}

void festation::Renderer::drawRectangle(const RectanglePrimitiveData &rectData)
{
    glm::vec4 color = {
        rectData.color.r / 255.0f,
        rectData.color.g / 255.0f,
        rectData.color.b / 255.0f,
        rectData.color.a,
    };

    m_vertices.append_range(std::array {
        PrimitiveVertex { glm::vec2(rectData.vertex1.x, rectData.vertex1.y), color },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y), color },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y + rectData.size.y), color },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x, rectData.vertex1.y + rectData.size.y), color },
    });

    m_indicesCount += MAX_INDICES_PER_PRIMITIVE;
}

void festation::Renderer::renderFrame()
{
    if (m_vertices.empty())
        return;

    glNamedBufferSubData(m_VBO, 0, sizeof(PrimitiveVertex) * m_vertices.size(), m_vertices.data());

    m_flatColorShader->apply();
    m_flatColorShader->setData("uProjection", m_projection);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, nullptr);

    m_vertices.clear();
}
