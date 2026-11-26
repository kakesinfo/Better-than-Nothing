#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 projection;
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

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 prevModel;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outColor;
layout(location = 4) out vec4 outCurrClipPos;
layout(location = 5) out vec4 outPrevClipPos;

void main()
{
    vec4 worldPos = pushConstants.model * vec4(inPosition, 1.0);
    vec4 prevWorldPos = pushConstants.prevModel * vec4(inPosition, 1.0);

    outWorldPos = worldPos.xyz;
    outWorldNormal = normalize(mat3(transpose(inverse(pushConstants.model))) * inNormal);
    outTexCoord = inTexCoord;
    outColor = inColor;
    outCurrClipPos = globalUniforms.unjitteredViewProj * worldPos;
    outPrevClipPos = globalUniforms.prevUnjitteredViewProj * prevWorldPos;

    gl_Position = globalUniforms.viewProj * worldPos;
}
