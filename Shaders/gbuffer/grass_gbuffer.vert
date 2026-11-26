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

vec3 ComputeWindOffset(vec2 worldXZ, float h, float phase, float t)
{
    float bend = h * h;
    float wave1 = sin(dot(worldXZ, vec2(0.18, 0.11)) + t * 2.1 + phase * 6.28318);
    float wave2 = cos(dot(worldXZ, vec2(-0.14, 0.22)) - t * 1.6 + phase * 3.14159);
    float flutter = sin(t * 5.2 + phase * 18.8495) * 0.18;

    vec2 windDir = normalize(vec2(0.82, 0.57));
    vec2 crossDir = vec2(-windDir.y, windDir.x);

    vec2 horiz = (windDir * (wave1 * 0.22 + 0.12 + flutter * 0.08) + crossDir * (wave2 * 0.10)) * bend;
    float vert = -min(length(horiz) * 0.38, 0.25 * h);
    return vec3(horiz.x, vert, horiz.y);
}

void main()
{
    float h = clamp(inTexCoord.y, 0.0, 1.0);
    float phase = inColor.x;
    float bladeHeight = max(inColor.z, 0.01);

    vec4 baseWorld = pushConstants.model * vec4(inPosition, 1.0);
    vec4 prevBaseWorld = pushConstants.prevModel * vec4(inPosition, 1.0);

    float distToCam = length(globalUniforms.cameraPosition - baseWorld.xyz);
    float fade = 1.0 - smoothstep(105.0, 145.0, distToCam);
    float verticalCollapse = (1.0 - fade) * h * bladeHeight;

    vec3 currWind = ComputeWindOffset(baseWorld.xz, h, phase, globalUniforms.elapsedTime);
    vec3 prevWind = ComputeWindOffset(prevBaseWorld.xz, h, phase, globalUniforms.elapsedTime - max(globalUniforms.deltaTime, 0.0166));

    vec4 currWorldPos = vec4(baseWorld.xyz + currWind * fade - vec3(0.0, verticalCollapse, 0.0), 1.0);
    vec4 prevWorldPos = vec4(prevBaseWorld.xyz + prevWind * fade - vec3(0.0, verticalCollapse, 0.0), 1.0);

    vec3 baseNormal = normalize(mat3(transpose(inverse(pushConstants.model))) * inNormal);
    vec3 windTiltedNormal = normalize(baseNormal + vec3(currWind.x * 0.85, 0.0, currWind.z * 0.85));

    outWorldPos = currWorldPos.xyz;
    outWorldNormal = windTiltedNormal;
    outTexCoord = inTexCoord;
    outColor = inColor;
    outCurrClipPos = globalUniforms.unjitteredViewProj * currWorldPos;
    outPrevClipPos = globalUniforms.prevUnjitteredViewProj * prevWorldPos;

    gl_Position = globalUniforms.viewProj * currWorldPos;
}
