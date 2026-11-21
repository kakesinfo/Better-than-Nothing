#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
    mat4 viewProj;
    mat4 invViewProj;
    mat4 unjitteredViewProj;
    mat4 prevUnjitteredViewProj;

    vec3 lightDirection;
    float sunAngularRadius;

    vec3 lightColor;
    float sunIlluminanceLux;

    vec3 cameraPosition;
    float cameraExposure;

    uint pointLightsCount;
    uint frameIndex;
    uint screenWidth;
    uint screenHeight;

    uint debugView;
    uint toneMapMode;
    uint flags;
    float taaBlendFactor;

    float skyLuminanceNits;
    float atmosphereAltitudeKm;
    float atmosphereMieAnisotropy;
    float atmosphereMultiScatteringFactor;

    float atmosphereDensityScale;
    float _pad0;
    float _pad1;
    float _pad2;
} globalUniforms;

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 prevModel;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outWorldNormal;
layout(location = 4) out vec4 outCurrClipPos;
layout(location = 5) out vec4 outPrevClipPos;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    vec4 prevWorldPos = push.prevModel * vec4(inPosition, 1.0);

    gl_Position = globalUniforms.viewProj * worldPos;

    outWorldPos = worldPos.xyz;
    outColor = inColor;
    outTexCoord = inTexCoord;
    outWorldNormal = normalize(mat3(transpose(inverse(push.model))) * inNormal);
    outCurrClipPos = globalUniforms.unjitteredViewProj * worldPos;
    outPrevClipPos = globalUniforms.prevUnjitteredViewProj * prevWorldPos;
}
