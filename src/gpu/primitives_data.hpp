#pragma once

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace festation {
    struct DrawingAreaInfo {
        glm::u16vec2 topLeft;
        glm::u16vec2 bottomRight;
        glm::i16vec2 offset;
    };

    inline constexpr size_t COLOR_PARAM_POS = 0; 
    inline constexpr size_t VERTEX_PARAM_POS = 1; 
    inline constexpr size_t UV_PARAM_POS = 2; 
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
            glm::u16vec2 uv;
            glm::u16vec2 clut;
        } clutUV;

        glm::u16vec2 size;
    };
};
