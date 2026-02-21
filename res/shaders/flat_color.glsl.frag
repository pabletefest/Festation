#version 460

layout(location = 0) out vec4 FragColor;

in vec4 vColor;

void main() {
    ivec3 roundedColor = ivec3(vColor.rgb * 31.0 + 0.5);
    vec3 color = vec3(roundedColor.rgb) / 31.0;
    FragColor = vec4(color, vColor.a);
}
