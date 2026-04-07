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

vec3 convertRGB5toRGB8Color(in vec3 rgb5Color) {
    vec3 rgb8Color = rgb5Color.rgb * 255.0 / 31.0;
    return rgb8Color.rgb / 255.0;
}

void main() {
    // vec3 color = roundToRGB5Color(vColor.rgb);
    vec4 finalColor = vec4(1.0);

    // @todo: try to improve this by making it branchless
    switch(vTexIndex) {
        case 0:
        {
            finalColor = vec4(roundToRGB5Color(vColor.rgb), vColor.a);
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

            float red = (clutColor & 0x1Fu) * 255.0 / 31.0;
            float green = ((clutColor >> 5u) & 0x1Fu) * 255.0 / 31.0;
            float blue = ((clutColor >> 10u) & 0x1Fu) * 255.0 / 31.0;
            
            vec3 pixelColor = vec3(red, green, blue);
            vec3 color = vColor.rgb * 255.0;
            finalColor.rgb = roundToRGB5Color((pixelColor.rgb * color.rgb) / vec3(128.0) / 255.0);
        }
            break;
        default:
            finalColor = vec4(roundToRGB5Color(vColor.rgb), vColor.a);
            break;
    }

    FragColor = vec4(finalColor.rgb, 1.0);
}
