#version 450

// Physically-Based Animated Water Surface Fragment Shader
// Features:
// - Gerstner wave normals + dual-scale animated procedural micro-ripples
// - Depth-buffer underwater reconstruction + Beer-Lambert spectral absorption
// - Screen-space refracted scene color behind water
// - Physically-based Schlick Fresnel (F0 = 0.02) + Hillaire IBL sky reflections
// - GGX sun specular highlights + backlit wave-crest subsurface scattering
// - Animated shoreline & crest foam + soft shoreline fade

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

layout(set = 1, binding = 0) uniform sampler2D depthTex;
layout(set = 1, binding = 1) uniform sampler2D sceneColorTex;
layout(set = 1, binding = 2) uniform sampler2D transmittanceLUT;
layout(set = 1, binding = 3) uniform sampler2D irradianceLUT;
layout(set = 1, binding = 4) uniform sampler2D prefilteredEnvLUT;
layout(set = 1, binding = 5) uniform sampler2D brdfLUT;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWaveNormal;
layout(location = 2) in vec2 inWorldXZ;
layout(location = 3) in float inWaveCrest;
layout(location = 4) in float inDepthHint;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float GROUND_RADIUS_KM = 6360.0;
const float ATMOSPHERE_RADIUS_KM = 6460.0;

float SafeSqrt(float x) {
    return sqrt(max(x, 0.0));
}

float DistanceToTopAtmosphereBoundary(float r, float mu) {
    float discriminant = r * r * (mu * mu - 1.0) + ATMOSPHERE_RADIUS_KM * ATMOSPHERE_RADIUS_KM;
    return max(0.0, -r * mu + SafeSqrt(discriminant));
}

vec2 TransmittanceUV(float r, float mu) {
    float H = SafeSqrt(ATMOSPHERE_RADIUS_KM * ATMOSPHERE_RADIUS_KM - GROUND_RADIUS_KM * GROUND_RADIUS_KM);
    float rho = SafeSqrt(r * r - GROUND_RADIUS_KM * GROUND_RADIUS_KM);
    float d = DistanceToTopAtmosphereBoundary(r, mu);
    float d_min = ATMOSPHERE_RADIUS_KM - r;
    float d_max = rho + H;
    return clamp(vec2((d - d_min) / max(d_max - d_min, 1e-6), rho / H), vec2(0.0), vec2(1.0));
}

vec2 DirectionToEquirectUV(vec3 dir) {
    float azimuth = atan(dir.z, dir.x);
    float u = azimuth / (2.0 * PI) + 0.5;
    float elevation = asin(clamp(dir.y, -1.0, 1.0));
    float v = elevation / PI + 0.5;
    return vec2(u, clamp(v, 0.015, 0.985));
}

vec3 SamplePrefilteredEnv(vec3 dir, float roughness) {
    const float numBands = 5.0;
    vec2 baseUV = DirectionToEquirectUV(dir);
    float scaled = clamp(roughness, 0.0, 1.0) * (numBands - 1.0);
    float band0 = floor(scaled);
    float band1 = min(band0 + 1.0, numBands - 1.0);
    float frac = scaled - band0;

    float localV = clamp(baseUV.y, 0.02, 0.98);
    vec3 c0 = textureLod(prefilteredEnvLUT, vec2(baseUV.x, (band0 + localV) / numBands), 0.0).rgb;
    vec3 c1 = textureLod(prefilteredEnvLUT, vec2(baseUV.x, (band1 + localV) / numBands), 0.0).rgb;
    return mix(c0, c1, frac);
}

vec3 ReconstructWorldPos(vec2 uv, float depth) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = globalUniforms.invViewProj * clipPos;
    return worldPos.xyz / worldPos.w;
}

float Hash21(vec2 p) {
    p = fract(p * vec2(127.1, 311.7));
    p += dot(p, p + 34.5);
    return fract(p.x * p.y);
}

float ValueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float RippleHeight(vec2 p, float t) {
    float h = 0.0;
    h += ValueNoise(p * 0.95 + vec2( t * 0.55,  t * 0.35)) * 0.50;
    h += ValueNoise(p * 2.15 + vec2(-t * 0.85,  t * 0.60)) * 0.30;
    h += ValueNoise(p * 4.80 + vec2( t * 1.15, -t * 0.95)) * 0.20;
    return h;
}

vec3 ComputeMicroRippleNormal(vec2 xz, float t, float distFade) {
    const float eps = 0.08;
    float hC = RippleHeight(xz, t);
    float hX = RippleHeight(xz + vec2(eps, 0.0), t);
    float hZ = RippleHeight(xz + vec2(0.0, eps), t);
    float strength = 0.14 * distFade;
    return normalize(vec3((hC - hX) / eps * strength, 1.0, (hC - hZ) / eps * strength));
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

void main() {
    // Bypass water rendering when inspecting G-Buffer / AO / Shadow debug views
    if (globalUniforms.debugView != 0u && globalUniforms.debugView != 11u)
        discard;

    vec2 screenUV = gl_FragCoord.xy / vec2(globalUniforms.screenWidth, globalUniforms.screenHeight);
    float sceneDepth = textureLod(depthTex, screenUV, 0.0).r;

    vec3 V_vec = globalUniforms.cameraPosition - inWorldPos;
    float viewDist = length(V_vec);
    vec3 V = V_vec / max(viewDist, 1e-5);

    // Compute true vertical & optical water depth from opaque scene depth buffer
    float waterDepth = inDepthHint;
    float opticalDepth = inDepthHint;
    if (sceneDepth < 0.99999) {
        vec3 sceneWorldPos = ReconstructWorldPos(screenUV, sceneDepth);
        waterDepth = max(inWorldPos.y - sceneWorldPos.y, 0.0);
        opticalDepth = max(length(sceneWorldPos - inWorldPos), 0.0);
    }

    // Soft shoreline fade where water meets sand/rocks
    float edgeAlpha = smoothstep(0.0, 0.16, waterDepth);
    if (edgeAlpha <= 0.001)
        discard;

    // Combine Gerstner normal + distance-faded micro-ripples
    float rippleFade = 1.0 - smoothstep(25.0, 140.0, viewDist);
    vec3 rippleN = ComputeMicroRippleNormal(inWorldXZ, globalUniforms.time, rippleFade);
    vec3 N = normalize(vec3(
        inWaveNormal.x + rippleN.x,
        max(inWaveNormal.y, 0.25),
        inWaveNormal.z + rippleN.z
    ));

    float NdotV = max(dot(N, V), 0.001);

    // Physical water Fresnel (IOR = 1.333 => F0 = 0.02)
    const float F0 = 0.02;
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    // Atmospheric sun illuminance at camera altitude
    float camAltKm = max(globalUniforms.atmosphereAltitudeKm + max(globalUniforms.cameraPosition.y * 0.001, 0.0), 0.005);
    float camR = min(GROUND_RADIUS_KM + camAltKm, ATMOSPHERE_RADIUS_KM - 0.05);
    vec3 sunDir = normalize(-globalUniforms.lightDirection);
    float sunHorizonFade = smoothstep(-0.04, 0.02, sunDir.y);
    vec3 sunTrans = textureLod(transmittanceLUT, TransmittanceUV(camR, sunDir.y), 0.0).rgb * sunHorizonFade;
    vec3 sunIlluminanceLux = globalUniforms.lightColor * globalUniforms.sunIlluminanceLux * sunTrans;

    // Screen-space refraction (perturbed UV clamped to underwater pixels so foreground objects never bleed in)
    vec2 refractOffset = N.xz * (0.024 * min(waterDepth, 1.5)) / max(viewDist * 0.12, 1.0);
    vec2 refractUV = clamp(screenUV + refractOffset, vec2(0.002), vec2(0.998));
    float refractDepth = textureLod(depthTex, refractUV, 0.0).r;
    if (refractDepth < gl_FragCoord.z) {
        refractUV = screenUV;
    }
    vec3 refractedPreExposed = textureLod(sceneColorTex, refractUV, 0.0).rgb;
    float invExposure = 1.0 / max(globalUniforms.cameraExposure, 1e-8);
    vec3 refractedNits = refractedPreExposed * invExposure;

    // Beer-Lambert spectral water absorption (red absorbs fastest, blue/teal penetrates deepest)
    vec3 absorptionCoeff = vec3(0.58, 0.22, 0.14);
    vec3 waterTransmittance = exp(-opticalDepth * absorptionCoeff);

    // Volumetric scatter color inside the water body (illuminated by sun + sky irradiance)
    vec3 skyIrradianceLux = textureLod(irradianceLUT, DirectionToEquirectUV(vec3(0.0, 1.0, 0.0)), 0.0).rgb * globalUniforms.irradianceIntensity;
    vec3 deepScatterAlbedo = vec3(0.012, 0.068, 0.088);
    vec3 shallowScatterAlbedo = vec3(0.04, 0.18, 0.17);
    vec3 waterBodyAlbedo = mix(shallowScatterAlbedo, deepScatterAlbedo, smoothstep(0.0, 3.2, waterDepth));
    vec3 waterBodyNits = (waterBodyAlbedo / PI) * (skyIrradianceLux + sunIlluminanceLux * max(sunDir.y, 0.0) * 0.35);

    // Backlit subsurface scattering glow through wave crests
    float sssFactor = pow(clamp(dot(V, -sunDir + N * 0.45), 0.0, 1.0), 4.0) * (0.35 + 0.65 * inWaveCrest);
    vec3 sssNits = vec3(0.03, 0.22, 0.18) * (sunIlluminanceLux / PI) * sssFactor * 0.45;

    vec3 underwaterNits = refractedNits * waterTransmittance + waterBodyNits * (1.0 - waterTransmittance) + sssNits;

    // Sky & environment reflection from Hillaire prefiltered environment
    vec3 R = reflect(-V, N);
    R.y = max(R.y, 0.02); // Keep reflection above horizon
    float waterRoughness = mix(0.045, 0.14, smoothstep(20.0, 140.0, viewDist));
    vec3 envReflectionNits = SamplePrefilteredEnv(R, waterRoughness) * globalUniforms.irradianceIntensity;

    // Direct GGX sun specular glints
    float NdotL = max(dot(N, sunDir), 0.0);
    vec3 sunSpecNits = vec3(0.0);
    if (NdotL > 0.0) {
        vec3 H = normalize(V + sunDir);
        float NDF = DistributionGGX(N, H, max(waterRoughness, 0.035));
        float G = GeometrySmith(N, V, sunDir, max(waterRoughness, 0.035));
        float F_sun = F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);
        sunSpecNits = ((NDF * G * F_sun) / max(4.0 * NdotV * NdotL, 1e-5)) * sunIlluminanceLux * NdotL;
    }

    vec3 waterSurfaceNits = mix(underwaterNits, envReflectionNits, fresnel) + sunSpecNits;

    // Animated shoreline foam + wave crest foam
    float foamNoise1 = ValueNoise(inWorldXZ * 1.4 + vec2(globalUniforms.time * 0.4, -globalUniforms.time * 0.25));
    float foamNoise2 = ValueNoise(inWorldXZ * 3.8 - vec2(globalUniforms.time * 0.6, globalUniforms.time * 0.5));
    float foamPattern = foamNoise1 * 0.6 + foamNoise2 * 0.4;

    float shoreWave = sin(waterDepth * 14.0 - globalUniforms.time * 2.8 + foamNoise1 * 4.0) * 0.5 + 0.5;
    float shoreFoam = (1.0 - smoothstep(0.04, 0.55, waterDepth)) * smoothstep(0.32, 0.72, foamPattern + shoreWave * 0.28);
    float crestFoam = smoothstep(0.62, 0.95, inWaveCrest * (0.75 + 0.5 * foamPattern)) * 0.45;
    float foam = clamp((shoreFoam + crestFoam) * globalUniforms.waterFoamIntensity, 0.0, 1.0);

    vec3 foamAlbedo = vec3(0.90, 0.94, 0.96);
    vec3 foamNits = (foamAlbedo / PI) * (skyIrradianceLux + sunIlluminanceLux * max(dot(N, sunDir), 0.0));

    vec3 finalNits = mix(waterSurfaceNits, foamNits, foam);
    vec3 preExposedHdr = clamp(finalNits * globalUniforms.cameraExposure, vec3(0.0), vec3(65000.0));
    outColor = vec4(preExposedHdr, edgeAlpha);
}
