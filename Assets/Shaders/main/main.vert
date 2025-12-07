#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
} globalUniforms;

layout(set = 1, binding = 0) uniform DynamicUniformBuffer {
    mat4 model;
} dynamicUniforms;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = globalUniforms.proj * globalUniforms.view * dynamicUniforms.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}