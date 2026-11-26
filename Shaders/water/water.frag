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

layout(set = 1, binding = 0) uniform sampler2D sceneColorTex;
layout(set = 1, binding = 1) uniform sampler2D sceneDepthTex;
layout(set = 1, binding = 2) uniform sampler2D skyViewLUT;
layout(set = 1, binding = 3) uniform sampler2D irradianceLUT;
layout(set = 1, binding = 4) uniform sampler2D prefilteredEnvLUT;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in float inShoreDepth;
layout(location = 4) in vec4 inClipPos;

layout(location = 0) out vec4 outHdrColor;

const float PI = 3.14159265359;

vec2 DirToEquirectUV(vec3 dir)
{
    float phi = atan(dir.z, dir.x);
    float theta = acos(clamp(dir.y, -1.0, 1.0));
    return vec2(phi / (2.0 * PI) + 0.5, theta / PI);
}

vec3 SamplePrefilteredSky(vec3 R, float roughness)
{
    vec2 eqUv = DirToEquirectUV(R);
    float band = clamp(roughness, 0.0, 1.0) * 4.0;
    float b0 = floor(band);
    float b1 = min(b0 + 1.0, 4.0);
    float frac = band - b0;

    float v0 = (b0 + clamp(eqUv.y, 0.02, 0.98)) / 5.0;
    float v1 = (b1 + clamp(eqUv.y, 0.02, 0.98)) / 5.0;
    vec3 c0 = textureLod(prefilteredEnvLUT, vec2(eqUv.x, v0), 0.0).rgb;
    vec3 c1 = textureLod(prefilteredEnvLUT, vec2(eqUv.x, v1), 0.0).rgb;
    return mix(c0, c1, frac);
}

vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = globalUniforms.invViewProj * clipPos;
    return worldPos.xyz / worldPos.w;
}

float Hash21(vec2 p)
{
    p = fract(p * vec2(127.1, 311.7));
    p += dot(p, p + 34.5);
    return fract(p.x * p.y);
}

float ValueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float RippleHeight(vec2 p, float t)
{
    float h = 0.0;
    h += ValueNoise(p * 0.95 + vec2(t * 0.55, t * 0.35)) * 0.50;
    h += ValueNoise(p * 2.15 + vec2(-t * 0.85, t * 0.60)) * 0.30;
    h += ValueNoise(p * 4.80 + vec2(t * 1.15, -t * 0.95)) * 0.20;
    return h;
}

vec3 ComputeMicroRippleNormal(vec2 xz, float t, float distFade)
{
    const float eps = 0.08;
    float hC = RippleHeight(xz, t);
    float hX = RippleHeight(xz + vec2(eps, 0.0), t);
    float hZ = RippleHeight(xz + vec2(0.0, eps), t);
    float strength = 0.30 * distFade;
    return vec3((hC - hX) / eps * strength, 1.0, (hC - hZ) / eps * strength);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 1e-6);
}

void main()
{
    vec2 screenUv = gl_FragCoord.xy / vec2(globalUniforms.screenWidth, globalUniforms.screenHeight);
    vec3 viewVec = inWorldPos - globalUniforms.cameraPosition;
    float viewDist = length(viewVec);
    vec3 V = -viewVec / max(viewDist, 1e-5);

    float bgDepth = textureLod(sceneDepthTex, screenUv, 0.0).r;
    float waterDepth = max(inShoreDepth, 0.05);
    if (bgDepth < 1.0)
    {
        vec3 bgPos = ReconstructWorldPos(screenUv, bgDepth);
        waterDepth = max(inWorldPos.y - bgPos.y, 0.0);
    }

    float rippleFade = 1.0 - smoothstep(25.0, 140.0, viewDist);
    vec3 rippleN = ComputeMicroRippleNormal(inWorldPos.xz, globalUniforms.elapsedTime, rippleFade);
    vec3 N = normalize(vec3(
        inWorldNormal.x + rippleN.x,
        max(inWorldNormal.y, 0.25),
        inWorldNormal.z + rippleN.z
    ));

    float NdotV = max(dot(N, V), 0.001);
    const float F0 = 0.02;
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    vec3 sunDir = normalize(-globalUniforms.lightDirection);
    float sunHorizonFade = smoothstep(-0.04, 0.02, sunDir.y);
    vec3 sunIlluminanceLux = globalUniforms.lightColor * globalUniforms.lightIntensity * sunHorizonFade;

    vec2 refractOffset = N.xz * (0.024 * min(waterDepth, 1.5)) / max(viewDist * 0.12, 1.0);
    vec2 refractUv = clamp(screenUv + refractOffset, vec2(0.002), vec2(0.998));
    if (textureLod(sceneDepthTex, refractUv, 0.0).r < gl_FragCoord.z)
        refractUv = screenUv;

    float invExposure = 1.0 / max(globalUniforms.cameraExposure, 1e-8);
    vec3 refractedNits = textureLod(sceneColorTex, refractUv, 0.0).rgb * invExposure;

    vec3 absorptionCoeff = vec3(0.58, 0.22, 0.14);
    float opticalDepth = waterDepth / max(NdotV, 0.2);
    vec3 waterTransmittance = exp(-opticalDepth * absorptionCoeff);

    vec3 skyIrradianceLux = textureLod(irradianceLUT, DirToEquirectUV(vec3(0.0, 1.0, 0.0)), 0.0).rgb;
    vec3 deepScatterAlbedo = vec3(0.012, 0.068, 0.088);
    vec3 shallowScatterAlbedo = vec3(0.04, 0.18, 0.17);
    vec3 waterBodyAlbedo = mix(shallowScatterAlbedo, deepScatterAlbedo, smoothstep(0.0, 3.2, waterDepth));
    vec3 waterBodyNits = (waterBodyAlbedo / PI) * (skyIrradianceLux + sunIlluminanceLux * max(sunDir.y, 0.0) * 0.35);

    vec3 underwaterNits = refractedNits * waterTransmittance + waterBodyNits * (1.0 - waterTransmittance);

    vec3 R = reflect(-V, N);
    R.y = max(R.y, 0.02);
    float waterRoughness = mix(0.045, 0.14, smoothstep(20.0, 140.0, viewDist));
    vec3 envReflectionNits = SamplePrefilteredSky(R, waterRoughness);

    float NdotL = max(dot(N, sunDir), 0.0);
    vec3 H = normalize(V + sunDir);
    float NDF = DistributionGGX(N, H, max(waterRoughness, 0.035));
    float F_sun = F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);
    vec3 sunSpecNits = ((NDF * F_sun) / max(4.0 * NdotV * NdotL + 0.001, 1e-4)) * sunIlluminanceLux * NdotL;

    vec3 waterSurfaceNits = mix(underwaterNits, envReflectionNits, fresnel) + sunSpecNits;

    float foamNoise1 = ValueNoise(inWorldPos.xz * 1.4 + vec2(globalUniforms.elapsedTime * 0.4, -globalUniforms.elapsedTime * 0.25));
    float foamNoise2 = ValueNoise(inWorldPos.xz * 3.8 - vec2(globalUniforms.elapsedTime * 0.6, globalUniforms.elapsedTime * 0.5));
    float foamPattern = foamNoise1 * 0.6 + foamNoise2 * 0.4;
    float shoreWave = sin(waterDepth * 14.0 - globalUniforms.elapsedTime * 2.8 + foamNoise1 * 4.0) * 0.5 + 0.5;
    float shoreFoam = (1.0 - smoothstep(0.04, 0.55, waterDepth)) * smoothstep(0.32, 0.72, foamPattern + shoreWave * 0.28);

    vec3 foamNits = (vec3(0.90, 0.94, 0.96) / PI) * (skyIrradianceLux + sunIlluminanceLux * NdotL);
    vec3 finalNits = mix(waterSurfaceNits, foamNits, clamp(shoreFoam * 0.8, 0.0, 1.0));

    vec3 preExposed = clamp(finalNits * globalUniforms.cameraExposure, vec3(0.0), vec3(65000.0));
    float alpha = clamp(smoothstep(0.0, 0.12, waterDepth), 0.0, 1.0);
    outHdrColor = vec4(preExposed, alpha);
}
