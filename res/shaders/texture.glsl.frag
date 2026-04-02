#version 460

layout(location = 0) out vec4 FragColor;

in vec4 vColor;
in vec2 vTexCoord;
flat in uint vTexIndex;
flat in uint vBppDepth;
flat in uvec2 vClut;

// layout (binding = 0) uniform sampler2D uTextures[2];
// layout (binding = 0) uniform sampler2D uWhiteTexture;
layout (binding = 0) uniform usampler2D uVramTexture;

void main() {
    ivec3 roundedColor = ivec3(vColor.rgb * 31.0 + 0.5);
    vec3 color = vec3(roundedColor.rgb) / 31.0;

    // @todo: try to improve this by making it branchless
    switch(vTexIndex) {
        case 0:
        {
            FragColor = vec4(color, vColor.a);
        }
            break;
        case 1:
        {
            uvec2 texPage = uvec2(vTexCoord);
            uint texel = texelFetch(uVramTexture, ivec2(texPage), 0).r;

            uvec2 clutIndex;
            uint texelOffset = (texPage.x & 3u) * 4u;
            uint clutOffset = ((texel >> texelOffset) & 0xFu);
            clutIndex.x = vClut.x * 16u + clutOffset;
            clutIndex.y = vClut.y;

            uint clutColor = texelFetch(uVramTexture, ivec2(clutIndex), 0).r;

            // if (clutColor == 0)
            //     discard;

            uint red = (clutColor << 3u) & 0xF8u;
            uint green = (clutColor >> 2u) & 0xF8u;
            uint blue = (clutColor >> 7u) & 0xF8u;
            float redF = float(red) / 255.0;
            float greenF = float(green) / 255.0;
            float blueF = float(blue) / 255.0;
            
            vec4 pixelColor = vec4(redF, greenF, blueF, 1.0);
            FragColor = pixelColor;

            // uint clutColor = texelFetch(uVramTexture, ivec2(clutIndex), 0).r;

            // uint r =  clutColor        & 0x1Fu;
            // uint g = (clutColor >> 5)  & 0x1Fu;
            // uint b = (clutColor >> 10) & 0x1Fu;

            // vec3 texColor = vec3(r, g, b) / 31.0;

            // FragColor = vec4(texColor, 1.0) * vec4(color, vColor.a);
        }
            break;
        default:
            FragColor = vec4(color, vColor.a);
            break;
    }
}
