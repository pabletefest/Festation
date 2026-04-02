#version 460

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in uint aTexIndex;
layout(location = 4) in uint aBppDepth;
layout(location = 5) in uvec2 aClut;

out vec4 vColor;
out vec2 vTexCoord;
flat out uint vTexIndex;
flat out uint vBppDepth;
flat out uvec2 vClut;

uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
    vTexIndex = aTexIndex;
    vBppDepth = aBppDepth;
    vClut = aClut;
}
