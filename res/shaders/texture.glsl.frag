#version 460

layout(location = 0) out vec4 FragColor;

in vec4 vColor;
in vec2 vTexCoord;
flat in uint vTexIndex;
flat in uint vBppDepth;
flat in uvec2 vTexpage;
flat in uvec2 vClut;

// layout (binding = 0) uniform sampler2D uTextures[2];
// layout (binding = 0) uniform sampler2D uWhiteTexture;
layout (binding = 0) uniform usampler2D uVramTexture;

vec3 roundToRGB5Color(in vec3 inColor) {
    ivec3 roundedColor = ivec3(inColor.rgb * 31.0 + 0.5);
    vec3 color = vec3(roundedColor.rgb) / 31.0;
    return color;
}

void main() {
    vec3 color = roundToRGB5Color(vColor.rgb);

    // @todo: try to improve this by making it branchless
    switch(vTexIndex) {
        case 0:
        {
            FragColor = vec4(color, vColor.a);
        }
            break;
        case 1:
        {
            uvec2 uv = uvec2(vTexCoord);
            uvec2 texelCoord;
            texelCoord.x = vTexpage.x * 64 + uv.x / 4u;
            texelCoord.y = vTexpage.y * 256 + uv.y;
            uint texel = texelFetch(uVramTexture, ivec2(texelCoord), 0).r;

            uvec2 clutIndex;
            uint texelOffset = (uv.x & 0x3u) * 4u;
            uint clutOffset = ((texel >> texelOffset) & 0xFu);
            clutIndex.x = vClut.x * 16u + clutOffset;
            clutIndex.y = vClut.y;

            uint clutColor = texelFetch(uVramTexture, ivec2(clutIndex), 0).r;

            if (clutColor == 0)
                discard;

            float red = ((clutColor >> 3u) & 0x1Fu) / 255.0;
            float green = (((clutColor >> 5u) >> 3u) & 0x1Fu) / 255.0;
            float blue = (((clutColor >> 10u) >> 3u) & 0x1Fu) / 255.0;
            
            vec4 pixelColor = vec4(vec3(red, green, blue), 1.0);
            FragColor = pixelColor * vec4(color, vColor.a);
        }
            break;
        default:
            FragColor = vec4(color, vColor.a);
            break;
    }
}
