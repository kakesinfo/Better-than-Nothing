#version 450

// Animated Gerstner-Wave Water Surface Vertex Shader

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
    float aoRadius;
    float aoPower;
    float irradianceIntensity;

    float time;
    float waterLevel;
    float waterWaveScale;
    float waterFoamIntensity;
} globalUniforms;

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 prevModel;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor; // x = local static water depth (waterLevel - terrainHeight)
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWaveNormal;
layout(location = 2) out vec2 outWorldXZ;
layout(location = 3) out float outWaveCrest;
layout(location = 4) out float outDepthHint;

const float PI = 3.14159265359;

// Evaluates a single Gerstner wave and accumulates displacement + tangent/bitangent derivatives
void AddGerstnerWave(vec4 wave, vec2 xz, float t, float ampScale, inout vec3 disp, inout vec3 tangent, inout vec3 bitangent) {
    vec2 dir = normalize(wave.xy);
    float steepness = wave.z * ampScale;
    float wavelength = wave.w;
    float k = 2.0 * PI / wavelength;
    float c = sqrt(9.81 / k);
    float f = k * (dot(dir, xz) - c * t);
    float a = steepness / k;

    float cosF = cos(f);
    float sinF = sin(f);

    disp.x += dir.x * (a * cosF);
    disp.y += a * sinF;
    disp.z += dir.y * (a * cosF);

    tangent += vec3(
        -dir.x * dir.x * (steepness * sinF),
        dir.x * (steepness * cosF),
        -dir.x * dir.y * (steepness * sinF)
    );
    bitangent += vec3(
        -dir.x * dir.y * (steepness * sinF),
        dir.y * (steepness * cosF),
        -dir.y * dir.y * (steepness * sinF)
    );
}

void main() {
    vec4 baseWorld = push.model * vec4(inPosition, 1.0);
    float depthHint = max(inColor.x, 0.0);

    // Smoothly attenuate wave height near the shoreline so waves never clip through dry ground
    float shoreAtten = smoothstep(0.05, 1.35, depthHint) * max(globalUniforms.waterWaveScale, 0.0);
    float t = globalUniforms.time;

    vec3 disp = vec3(0.0);
    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 bitangent = vec3(0.0, 0.0, 1.0);

    // 4 overlapping swells: (dir.x, dir.y, steepness, wavelength in meters)
    AddGerstnerWave(vec4( 0.85,  0.52, 0.085, 11.5), baseWorld.xz, t, shoreAtten, disp, tangent, bitangent);
    AddGerstnerWave(vec4(-0.40,  0.91, 0.065,  7.2), baseWorld.xz, t, shoreAtten, disp, tangent, bitangent);
    AddGerstnerWave(vec4( 0.25, -0.96, 0.048,  4.3), baseWorld.xz, t, shoreAtten, disp, tangent, bitangent);
    AddGerstnerWave(vec4(-0.92, -0.38, 0.032,  2.6), baseWorld.xz, t, shoreAtten, disp, tangent, bitangent);

    vec3 worldPos = baseWorld.xyz + disp;
    vec3 waveNormal = normalize(cross(bitangent, tangent));
    if (waveNormal.y < 0.0)
        waveNormal = -waveNormal;

    gl_Position = globalUniforms.viewProj * vec4(worldPos, 1.0);

    outWorldPos = worldPos;
    outWaveNormal = waveNormal;
    outWorldXZ = worldPos.xz;
    outWaveCrest = clamp(disp.y * 4.5, 0.0, 1.0);
    outDepthHint = depthHint;
}
