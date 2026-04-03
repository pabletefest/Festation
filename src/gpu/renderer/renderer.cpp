#include "renderer.hpp"
#include "hw/OpenGL/ogl_shader.hpp"

#include <filesystem>
#include <array>
#include <cstddef>

namespace festation {
    static constexpr size_t MAX_VERTICES_PER_PRIMITIVE = 4;
    static constexpr size_t MAX_INDICES_PER_PRIMITIVE = 6;
    static constexpr size_t MAX_PRIMITIVES_COUNT = 180'000; /** @brief Max theoretical count for the HW */
    static constexpr size_t MAX_VERTICES_COUNT = MAX_PRIMITIVES_COUNT * MAX_VERTICES_PER_PRIMITIVE;
    static constexpr size_t MAX_INDICES_COUNT = MAX_PRIMITIVES_COUNT * MAX_INDICES_PER_PRIMITIVE;
    static constexpr size_t INDICES_PER_TRIANGLE = 3;
    static constexpr size_t INDICES_PER_QUAD = 6;

    static constexpr size_t VRAM_WIDTH = 1024;
    static constexpr size_t VRAM_HEIGHT = 512;
    static constexpr glm::u16vec2 VRAM_SIZE = { VRAM_WIDTH, VRAM_HEIGHT };
};

festation::Renderer::Renderer(const std::vector<uint16_t>& vram)
    : m_flatColorShader(IShader::createUnique(SHADERS_PATH / "flat_color.glsl.vert",
        SHADERS_PATH / "flat_color.glsl.frag")), m_textureShader(IShader::createUnique(SHADERS_PATH / "texture.glsl.vert",
            SHADERS_PATH / "texture.glsl.frag")), m_VAO(0), m_VBO(0), m_IBO(0), m_projection(glm::mat4(1)), m_indicesCount(0), m_vramRef(vram)
{
    m_vramFramebuffer = IFramebuffer::createUnique({
        .size = VRAM_SIZE,
        .format = FboFormats::RGBA5_REV,
    });

    m_defaultWhiteTexture = ITexture::createUnique({
        .size = { 1, 1 },
        .format = TextureFormat::RGB5_REV,
        .filtering = TextureFiltering::NEAREST,
        .useMipmaps = false,
    });

    m_vramRawTexture = ITexture::createUnique(TextureInfo {
        .size = VRAM_SIZE,
        .format = TextureFormat::R16UI,
        .filtering = TextureFiltering::NEAREST,
        .useMipmaps = false,
    });

    uint16_t whiteColor = 0xFFFF;
    m_defaultWhiteTexture->setData((uint8_t *)&whiteColor, { 0, 0 }, { 1, 1 });

    m_vertices.reserve(MAX_VERTICES_PER_PRIMITIVE * MAX_PRIMITIVES_COUNT);
    m_indices.resize(MAX_INDICES_COUNT);
    // size_t offset = 0;

    // for (size_t i = 0; i < MAX_INDICES_COUNT; i += 6) {
    //     m_indices[i + 0] = 0 + offset;
    //     m_indices[i + 1] = 1 + offset;
    //     m_indices[i + 2] = 2 + offset;
    //     m_indices[i + 3] = 2 + offset;
    //     m_indices[i + 4] = 3 + offset;
    //     m_indices[i + 5] = 0 + offset;

    //     offset += 4;
    // }

    glCreateVertexArrays(1, &m_VAO);

    glCreateBuffers(1, &m_VBO);
    glNamedBufferStorage(m_VBO, sizeof(PrimitiveVertex) * MAX_VERTICES_PER_PRIMITIVE * MAX_PRIMITIVES_COUNT, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &m_IBO);
    glNamedBufferStorage(m_IBO, sizeof(GLuint) * m_indices.size(), m_indices.data(), GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(PrimitiveVertex));
    glVertexArrayElementBuffer(m_VAO, m_IBO);

    glEnableVertexArrayAttrib(m_VAO, 0);
    glEnableVertexArrayAttrib(m_VAO, 1);
    glEnableVertexArrayAttrib(m_VAO, 2);
    glEnableVertexArrayAttrib(m_VAO, 3);
    glEnableVertexArrayAttrib(m_VAO, 4);
    glEnableVertexArrayAttrib(m_VAO, 5);

    glVertexArrayAttribFormat(m_VAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(PrimitiveVertex, position));
    glVertexArrayAttribFormat(m_VAO, 1, 4, GL_FLOAT, GL_FALSE, offsetof(PrimitiveVertex, color));
    glVertexArrayAttribFormat(m_VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(PrimitiveVertex, texCoord));
    glVertexArrayAttribIFormat(m_VAO, 3, 1, GL_UNSIGNED_INT, offsetof(PrimitiveVertex, texIndex));
    glVertexArrayAttribIFormat(m_VAO, 4, 1, GL_UNSIGNED_INT, offsetof(PrimitiveVertex, bppDepth));
    glVertexArrayAttribIFormat(m_VAO, 5, 2, GL_UNSIGNED_INT, offsetof(PrimitiveVertex, clut));

    glVertexArrayAttribBinding(m_VAO, 0, 0);
    glVertexArrayAttribBinding(m_VAO, 1, 0);
    glVertexArrayAttribBinding(m_VAO, 2, 0);
    glVertexArrayAttribBinding(m_VAO, 3, 0);
    glVertexArrayAttribBinding(m_VAO, 4, 0);
    glVertexArrayAttribBinding(m_VAO, 5, 0);

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

auto festation::Renderer::setClearColor(const glm::vec4 &color) -> void
{
    glClearColor(color.r, color.g, color.b, color.a);
}

auto festation::Renderer::clearDisplay() -> void
{
    glClear(GL_COLOR_BUFFER_BIT);
}

auto festation::Renderer::setViewport(const glm::uvec2 &startCoord, const glm::uvec2 &size) -> void
{
    glViewport(startCoord.x, startCoord.y, size.x, size.y);
}

auto festation::Renderer::setClipRegion(const glm::uvec2 &startCoord, const glm::uvec2 &size) -> void
{
    glScissor(startCoord.x, startCoord.y, size.x, size.y);
}

auto festation::Renderer::setProjection(const glm::mat4 &projection) -> void
{
    m_projection = projection;
}

auto festation::Renderer::enableDepthTesting() -> void
{
    glEnable(GL_DEPTH_TEST);
}

auto festation::Renderer::disableDepthTesting() -> void
{
    glDisable(GL_DEPTH_TEST);
}

auto festation::Renderer::enableBlending() -> void
{
    glEnable(GL_BLEND);
}

auto festation::Renderer::disableBlending() -> void
{
    glDisable(GL_BLEND);
}

auto festation::Renderer::uploadVramToGpu(const uint8_t* data, const glm::uvec2 &offset = { 0, 0 }, const glm::uvec2 &size = VRAM_SIZE) -> void
{
    size_t stride = size.x * sizeof(uint16_t);

    for (int y = 0; y < size.y / 2; y++) {
        uint8_t* topLine = (uint8_t *)data + y * stride;
        uint8_t* bottomLine = (uint8_t *)data + (size.y - y - 1) * stride;

        for (int i = 0; i < stride; i++) {
            std::swap(topLine[i], bottomLine[i]);
        }
    }

    m_vramFramebuffer->setData(data, offset, size);
}

auto festation::Renderer::uploadVramToGpu(std::span<uint8_t> data, const glm::uvec2 &offset = { 0, 0 }, const glm::uvec2 &size = VRAM_SIZE) -> void
{
    size_t stride = size.x * sizeof(uint16_t);

    for (int y = 0; y < size.y / 2; y++) {
        uint8_t* topLine = data.data() + y * stride;
        uint8_t* bottomLine = data.data() + (size.y - y - 1) * stride;

        for (int i = 0; i < stride; i++) {
            std::swap(topLine[i], bottomLine[i]);
        }
    }
    
    m_vramFramebuffer->setData(data, offset, size);
}

void festation::Renderer::drawRectangle(const RectanglePrimitiveData &rectData)
{
    glm::vec4 color = {
        rectData.color.r / 255.0f,
        rectData.color.g / 255.0f,
        rectData.color.b / 255.0f,
        rectData.color.a,
    };

    m_indices[m_indicesCount + 0] = 0 + m_vertices.size();
    m_indices[m_indicesCount + 1] = 1 + m_vertices.size();
    m_indices[m_indicesCount + 2] = 2 + m_vertices.size();
    m_indices[m_indicesCount + 3] = 2 + m_vertices.size();
    m_indices[m_indicesCount + 4] = 3 + m_vertices.size();
    m_indices[m_indicesCount + 5] = 0 + m_vertices.size();

    m_vertices.append_range(std::array {
        PrimitiveVertex { glm::vec2(rectData.vertex1.x, rectData.vertex1.y), color, { 0.0f, 0.0f }, 0 },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y), color, { 1.0f, 0.0f }, 0 },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x + rectData.size.x, rectData.vertex1.y + rectData.size.y), color, { 1.0f, 1.0f }, 0 },
        PrimitiveVertex { glm::vec2(rectData.vertex1.x, rectData.vertex1.y + rectData.size.y), color, { 0.0f, 1.0f }, 0 },
    });

    m_indicesCount += INDICES_PER_QUAD;
}

auto festation::Renderer::drawRectangleTextured(const RectanglePrimitiveData &rectData, TexturePageColorsDepth colorDepth, 
    const std::vector<uint16_t>& vram) -> void
{
}

auto festation::Renderer::drawPolygon(const PolygonPrimitiveData &polygonData) -> void
{
    if (m_vertices.size() >= MAX_VERTICES_PER_PRIMITIVE * MAX_PRIMITIVES_COUNT) {
        renderBatch();
    }

    switch (polygonData.verticesCount)
    {
    case 3:
        m_indices[m_indicesCount + 0] = 0 + m_vertices.size();
        m_indices[m_indicesCount + 1] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 2] = 2 + m_vertices.size();

        m_indicesCount += INDICES_PER_TRIANGLE;
        break;
    case 4:
        m_indices[m_indicesCount + 0] = 0 + m_vertices.size();
        m_indices[m_indicesCount + 1] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 2] = 2 + m_vertices.size();
        m_indices[m_indicesCount + 3] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 4] = 2 + m_vertices.size();
        m_indices[m_indicesCount + 5] = 3 + m_vertices.size();

        m_indicesCount += INDICES_PER_QUAD;
        break;
    default:
        std::unreachable();
    }

    for (size_t vertexId = 0; vertexId < polygonData.verticesCount; vertexId++) {
        glm::vec2 texCoords;
        
        switch (vertexId)
        {
        case 0:
            texCoords.x = 0.0f;
            texCoords.y = 0.0f;
            break;
        case 1:
            texCoords.x = 1.0f;
            texCoords.y = 0.0f;
            break;
        case 2:
            if (polygonData.verticesCount == 4)  {
                texCoords.x = 1.0f;
                texCoords.y = 1.0f;
            }
            else {
                texCoords.x = 0.5f;
                texCoords.y = 1.0f;
            }

            break;
        case 3:
            texCoords.x = 0.0f;
            texCoords.y = 1.0f;
            break;
        default:
            std::unreachable();
        }

        m_vertices.emplace_back(PrimitiveVertex { 
            .position = glm::vec2 {
                polygonData.vertices[vertexId].x,
                polygonData.vertices[vertexId].y, 
            },
            .color = glm::vec4 {
                polygonData.colors[vertexId].r / 255.0f,
                polygonData.colors[vertexId].g / 255.0f,
                polygonData.colors[vertexId].b / 255.0f,
                polygonData.colors[vertexId].a,
            },
            .texCoord = texCoords,
            .texIndex = 0,
            .bppDepth = 0,  // Ignored for untextured polygons
            .clut = {}, // Ignored for untextured polygons 
        });
    }
}

auto festation::Renderer::drawPolygonTextured(const PolygonPrimitiveData &polygonData, TexturePageColorsDepth colorDepth, 
    const std::vector<uint16_t>& vram) -> void
{
    if (m_vertices.size() >= MAX_VERTICES_PER_PRIMITIVE * MAX_PRIMITIVES_COUNT) {
        renderBatch();
    }

    switch (polygonData.verticesCount)
    {
    case 3:
        m_indices[m_indicesCount + 0] = 0 + m_vertices.size();
        m_indices[m_indicesCount + 1] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 2] = 2 + m_vertices.size();

        m_indicesCount += INDICES_PER_TRIANGLE;
        break;
    case 4:
        m_indices[m_indicesCount + 0] = 0 + m_vertices.size();
        m_indices[m_indicesCount + 1] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 2] = 2 + m_vertices.size();
        m_indices[m_indicesCount + 3] = 1 + m_vertices.size();
        m_indices[m_indicesCount + 4] = 2 + m_vertices.size();
        m_indices[m_indicesCount + 5] = 3 + m_vertices.size();

        m_indicesCount += INDICES_PER_QUAD;
        break;
    default:
        std::unreachable();
    }

    for (size_t vertexId = 0; vertexId < polygonData.verticesCount; vertexId++) {
        glm::vec4 vertexColor;

        if (polygonData.isRawTexture) {
            vertexColor = glm::vec4 {
                1.0f,
                1.0f,
                1.0f,
                polygonData.colors[vertexId].a,
            };
        }
        else {
            vertexColor = glm::vec4 {
                polygonData.colors[vertexId].r / 255.0f,
                polygonData.colors[vertexId].g / 255.0f,
                polygonData.colors[vertexId].b / 255.0f,
                polygonData.colors[vertexId].a,
            };
        }

        glm::vec2 texCoords;
        uint32_t bppDepth;

        switch (colorDepth)
        {
        case Color4bit:
        {
            glm::u16vec2 texelIndex {
                polygonData.page.x * 64 + (polygonData.uvs[vertexId].x / 4),
                polygonData.page.y * 256 + polygonData.uvs[vertexId].y,
            };

            texCoords = texelIndex;
            bppDepth = 4;
        }
            break;
        case Color8bit:
            bppDepth = 8;
            break;
        case Color15bit:
        case ColorReserved:
            texCoords = (polygonData.page * glm::u16vec2(256, 256) + (glm::u16vec2)polygonData.uvs[vertexId]) / VRAM_SIZE;
            bppDepth = 15;
            break;
        default:
            std::unreachable();
        }

        m_vertices.emplace_back(PrimitiveVertex { 
            .position = glm::vec2 {
                polygonData.vertices[vertexId].x,
                polygonData.vertices[vertexId].y, 
            },
            .color = vertexColor,
            .texCoord = texCoords,
            .texIndex = 1,
            .bppDepth = bppDepth,
            .clut = polygonData.clut,
        });
    }
}

auto festation::Renderer::renderBatch() -> void
{
    if (!m_vertices.empty()) {
        m_vramFramebuffer->apply();
        glNamedBufferSubData(m_VBO, 0, sizeof(PrimitiveVertex) * m_vertices.size(), m_vertices.data());
        glNamedBufferSubData(m_IBO, 0, sizeof(GLuint) * m_indices.size(), m_indices.data());
        // glCopyTextureSubImage2D(m_vramCopyTexture->getHandle(), 0, 0, 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
        glTextureSubImage2D(m_vramRawTexture->getHandle(), 0, 0, 0, VRAM_WIDTH, VRAM_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_SHORT, m_vramRef.data());

        m_textureShader->apply();
        // constexpr uint32_t whiteTextureSlot = 0;
        constexpr uint32_t vramTextureSlot = 0;
        // m_defaultWhiteTexture->apply(whiteTextureSlot);
        m_vramRawTexture->apply(vramTextureSlot);
        m_textureShader->setData("uProjection", m_projection);
        // m_textureShader->setData("uTexture", textureSlot);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, nullptr);
        
        m_vertices.clear();
        m_indicesCount = 0;
    }

    glDisable(GL_SCISSOR_TEST);
    m_vramFramebuffer->blitToSwapchain();
    glEnable(GL_SCISSOR_TEST);
}
