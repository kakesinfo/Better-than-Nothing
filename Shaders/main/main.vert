#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
    mat4 viewProj;
    mat4 invViewProj;
    mat4 unjitteredViewProj;
    mat4 prevUnjitteredViewProj;

    vec3 lightDirection;
    float lightIntensity;

    vec3 lightColor;
    float cameraExposure;

    vec3 cameraPosition;
    uint pointLightCount;

    float screenWidth;
    float screenHeight;
    uint frameIndex;
    uint debugView;

    uint toneMapMode;
    uint flags;
    float elapsedTime;
    float deltaTime;
} globalUniforms;

layout(push_constant) uniform constants {
    mat4 model;
    mat4 prevModel;
} dynamicUniforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragNormal;

void main()
{
    vec4 worldPos = dynamicUniforms.model * vec4(inPosition, 1.0);
    gl_Position = globalUniforms.viewProj * worldPos;

    fragPosition = worldPos.xyz;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(dynamicUniforms.model))) * inNormal;
}
