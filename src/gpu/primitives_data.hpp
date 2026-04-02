#pragma once

#include <cstdint>
#include <array>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace festation {
    struct DrawingAreaInfo {
        glm::u16vec2 topLeft;
        glm::u16vec2 bottomRight;
        glm::i16vec2 offset;
    };

    inline constexpr size_t RECT_COLOR_PARAM_POS = 0; 
    inline constexpr size_t RECT_VERTEX_PARAM_POS = 1; 
    inline constexpr size_t RECT_UV_PARAM_POS = 2; 
    inline constexpr size_t RECT_SIZE_PARAM_POS = 3;

    struct RectanglePrimitiveData {
        enum RectSizeType {
        Variable,
        SinglePixel,
        Sprite8x8,
        Sprite16x16,
        } sizeType;

        bool isTextured;
        bool isSemiTransparent;
        bool isRawTexture; 
        glm::u8vec4 color;
        glm::i16vec2 vertex1;

        struct {
            glm::u8vec2 uv;
            glm::u16vec2 clut;
        } clutUV;

        glm::u16vec2 size;
    };

    struct PolygonPrimitiveData {
        bool isGouraudShading;
        uint8_t verticesCount;
        bool isTextured;
        bool isSemiTransparent;
        bool isRawTexture; 
        std::array<glm::u8vec4, 4> colors;
        std::array<glm::i16vec2, 4> vertices;
        std::array<glm::u8vec2, 4> uvs;
        glm::u16vec2 clut;
        glm::u16vec2 page;
    };

    struct PrimitiveVertex {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        uint32_t texIndex;
        uint32_t bppDepth;
        glm::uvec2 clut;
    };
};
