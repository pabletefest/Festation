#version 460

layout(location = 0) out vec4 FragColor;

uniform vec4 uColor;

void main() {
    ivec3 roundedColor = uColor.rgb * 31.0 + 0.5;
    vec3 color = vec3(roundedColor.rgb / 31.0);
    FragColor = vec4(color, uColor.a);
}
