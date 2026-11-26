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

    float irradianceIntensity;
    float waterWaveScale;
    float waterFoamIntensity;
    float grassWindStrength;

    float grassWindSpeed;
    float grassFadeDistance;
    float cloudDensity;
    float cloudShadowStrength;
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
layout(location = 3) out float outShoreDepth;
layout(location = 4) out vec4 outClipPos;

vec3 GerstnerWave(vec2 pos, vec2 dir, float steepness, float wavelength, float time, inout vec3 tangent, inout vec3 binormal)
{
    float k = 6.2831853 / wavelength;
    float c = sqrt(9.8 / k);
    vec2 d = normalize(dir);
    float f = k * (dot(d, pos) - c * time);
    float a = steepness / k;

    tangent += vec3(
        -d.x * d.x * (steepness * sin(f)),
        d.x * (steepness * cos(f)),
        -d.x * d.y * (steepness * sin(f))
    );
    binormal += vec3(
        -d.x * d.y * (steepness * sin(f)),
        d.y * (steepness * cos(f)),
        -d.y * d.y * (steepness * sin(f))
    );

    return vec3(d.x * (a * cos(f)), a * sin(f), d.y * (a * cos(f)));
}

void main()
{
    vec4 baseWorld = pushConstants.model * vec4(inPosition, 1.0);
    float shoreDepth = inColor.x;
    float damp = smoothstep(0.0, 0.75, shoreDepth);

    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);
    vec3 offset = vec3(0.0);

    float t = globalUniforms.elapsedTime;
    float waveAmp = damp * max(globalUniforms.waterWaveScale, 0.0);
    offset += GerstnerWave(baseWorld.xz, vec2(1.0, 0.35), 0.14 * waveAmp, 9.5, t, tangent, binormal);
    offset += GerstnerWave(baseWorld.xz, vec2(0.4, 0.9), 0.10 * waveAmp, 5.8, t, tangent, binormal);
    offset += GerstnerWave(baseWorld.xz, vec2(-0.7, 0.5), 0.07 * waveAmp, 3.4, t, tangent, binormal);
    offset += GerstnerWave(baseWorld.xz, vec2(0.2, -0.85), 0.05 * waveAmp, 2.1, t, tangent, binormal);

    vec4 worldPos = vec4(baseWorld.xyz + offset, 1.0);
    outWorldPos = worldPos.xyz;
    outWorldNormal = normalize(cross(binormal, tangent));
    outTexCoord = inTexCoord;
    outShoreDepth = shoreDepth;
    outClipPos = globalUniforms.viewProj * worldPos;
    gl_Position = outClipPos;
}
