#version 450

// Procedural 3D Grass Blade Vertex Shader
// - Travelling multi-frequency wind field + gust waves
// - Length-preserving quadratic blade bending (h^2)
// - Smooth distance scale fade (zero far overdraw or sub-pixel shimmer)
// - Analytical previous-frame wind position for ghost-free TAA motion vectors

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

    float grassWindStrength;
    float grassWindSpeed;
    float grassFadeDistance;
    float _padGrass;
} globalUniforms;

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 prevModel;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;    // r = hue variation, g = stiffness, b = random phase
layout(location = 2) in vec2 inTexCoord; // x = across blade [0..1], y = normalized blade height h [0..1]
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outWorldNormal;
layout(location = 4) out vec4 outCurrClipPos;
layout(location = 5) out vec4 outPrevClipPos;

vec3 EvaluateWindOffset(vec2 worldXZ, float h, float stiffness, float phase, float t) {
    float bend = h * h;
    float speed = max(globalUniforms.grassWindSpeed, 0.0);
    float strength = max(globalUniforms.grassWindStrength, 0.0) * mix(0.65, 1.25, 1.0 - stiffness);

    float wTime = t * speed;
    // Primary rolling meadow wind wave + secondary cross-gust + high-frequency blade flutter
    float wave1 = sin(dot(worldXZ, vec2(0.18, 0.11)) + wTime * 2.1 + phase * 6.28318);
    float wave2 = cos(dot(worldXZ, vec2(-0.14, 0.22)) - wTime * 1.6 + phase * 3.14159);
    float flutter = sin(wTime * 5.2 + phase * 18.8495) * 0.18;

    vec2 windDir = normalize(vec2(0.82, 0.57));
    vec2 crossDir = vec2(-windDir.y, windDir.x);

    vec2 horiz = (windDir * (wave1 * 0.22 + 0.12 + flutter * 0.08) + crossDir * (wave2 * 0.10)) * strength * bend;
    // Slight vertical drop when bent to preserve blade arc length
    float vert = -min(length(horiz) * 0.38, 0.25 * h);
    return vec3(horiz.x, vert, horiz.y);
}

void main() {
    vec4 baseWorld = push.model * vec4(inPosition, 1.0);
    vec4 prevBaseWorld = push.prevModel * vec4(inPosition, 1.0);

    float h = clamp(inTexCoord.y, 0.0, 1.0);
    float stiffness = clamp(inColor.g, 0.0, 1.0);
    float phase = inColor.b;

    // Smoothly sink/shrink distant blades into the terrain beyond fadeStart..fadeEnd
    float fadeEnd = max(globalUniforms.grassFadeDistance, 15.0);
    float fadeStart = fadeEnd * 0.62;
    float distToCam = length(globalUniforms.cameraPosition - baseWorld.xyz);
    float distScale = 1.0 - smoothstep(fadeStart, fadeEnd, distToCam);

    vec3 currWind = EvaluateWindOffset(baseWorld.xz, h, stiffness, phase, globalUniforms.time);
    vec3 prevWind = EvaluateWindOffset(prevBaseWorld.xz, h, stiffness, phase, globalUniforms.time - 0.0166);

    // Collapse blade height toward root when fading out at distance
    float verticalCollapse = (1.0 - distScale) * h * 0.55;
    vec3 currWorldPos = baseWorld.xyz + currWind * distScale - vec3(0.0, verticalCollapse, 0.0);
    vec3 prevWorldPos = prevBaseWorld.xyz + prevWind * distScale - vec3(0.0, verticalCollapse, 0.0);

    // Tilt normal slightly with wind bend for dynamic sunlit meadow waves
    vec3 baseNormal = normalize(mat3(transpose(inverse(push.model))) * inNormal);
    vec3 windTiltedNormal = normalize(baseNormal + vec3(currWind.x * 0.85, 0.0, currWind.z * 0.85));

    gl_Position = globalUniforms.viewProj * vec4(currWorldPos, 1.0);

    outWorldPos = currWorldPos;
    outColor = inColor;
    outTexCoord = inTexCoord;
    outWorldNormal = windTiltedNormal;
    outCurrClipPos = globalUniforms.unjitteredViewProj * vec4(currWorldPos, 1.0);
    outPrevClipPos = globalUniforms.prevUnjitteredViewProj * vec4(prevWorldPos, 1.0);
}
