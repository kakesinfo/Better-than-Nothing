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

vec3 ComputeWindOffset(vec3 worldPos, float heightFactor, float phase, float time)
{
    float gust = sin(worldPos.x * 0.25 + worldPos.z * 0.18 + time * 1.6);
    float sway = sin(worldPos.x * 1.1 + time * 2.8 + phase * 6.2831853);
    float flutter = cos(worldPos.z * 2.3 + time * 4.5 + phase * 12.56637);
    float bend = (gust * 0.08 + sway * 0.06 + flutter * 0.025) * (heightFactor * heightFactor);
    return vec3(bend, -abs(bend) * 0.25, bend * 0.6);
}

void main()
{
    float heightFactor = inTexCoord.y;
    float phase = inColor.x;
    float bladeHeight = max(inColor.z, 0.01);

    vec4 baseWorld = pushConstants.model * vec4(inPosition, 1.0);
    vec4 prevBaseWorld = pushConstants.prevModel * vec4(inPosition, 1.0);

    float distToCam = length(globalUniforms.cameraPosition - baseWorld.xyz);
    float fade = 1.0 - smoothstep(115.0, 145.0, distToCam);
    float rootOffset = heightFactor * bladeHeight * (1.0 - fade);
    baseWorld.y -= rootOffset;
    prevBaseWorld.y -= rootOffset;

    vec3 currWind = ComputeWindOffset(baseWorld.xyz, heightFactor, phase, globalUniforms.elapsedTime);
    vec3 prevWind = ComputeWindOffset(prevBaseWorld.xyz, heightFactor, phase, globalUniforms.elapsedTime - globalUniforms.deltaTime);

    vec4 currWorldPos = vec4(baseWorld.xyz + currWind * fade, 1.0);
    vec4 prevWorldPos = vec4(prevBaseWorld.xyz + prevWind * fade, 1.0);

    outWorldPos = currWorldPos.xyz;
    outWorldNormal = normalize(mat3(transpose(inverse(pushConstants.model))) * inNormal);
    outTexCoord = inTexCoord;
    outColor = inColor;
    outCurrClipPos = globalUniforms.unjitteredViewProj * currWorldPos;
    outPrevClipPos = globalUniforms.prevUnjitteredViewProj * prevWorldPos;

    gl_Position = globalUniforms.viewProj * currWorldPos;
}
