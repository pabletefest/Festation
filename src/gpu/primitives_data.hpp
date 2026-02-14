#pragma once

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

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
    glm::vec4 color;
    glm::i16vec2 vectex1;

    struct {
        glm::i16vec2 clut;
        glm::i16vec2 uv;
    } clutUV;

    glm::i16vec2 size;
};