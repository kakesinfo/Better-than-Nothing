#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
    vec3 lightDirection;
    vec3 lightColor;
    float lightIntensity;
    vec3 cameraPosition;
    float cameraExposure;
    int pointLightsCount;
} globalUniforms;

layout(push_constant) uniform constants {
    mat4 model;
} dynamicUniforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec4 fragTangent;

void main() {
    gl_Position = globalUniforms.proj * globalUniforms.view * dynamicUniforms.model * vec4(inPosition, 1.0);

    fragPosition = vec3(dynamicUniforms.model * vec4(inPosition, 1.0));
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    
    mat3 modelMatrix3 = mat3(dynamicUniforms.model);
    fragNormal = normalize(modelMatrix3 * inNormal);
    fragTangent = vec4(normalize(modelMatrix3 * inTangent.xyz), inTangent.w);
}