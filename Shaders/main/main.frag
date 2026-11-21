#version 450

// Forward Transparent PBR Pass with Irradiance Lighting (Split-Sum IBL) & Fixed Vulkan Jacobian Normal Mapping

struct PointLightData {
    vec3 position;
    float radius;
    vec3 color;
    float luminousPowerLm;
};

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
} globalUniforms;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 1, binding = 3) uniform MaterialUniformBuffer {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    float _pad;
} material;

layout(std430, set = 2, binding = 0) readonly buffer PointLightBuffer {
    PointLightData lights[];
} pointLights;

layout(set = 3, binding = 0) uniform sampler2D transmittanceLUT;
layout(set = 3, binding = 1) uniform sampler2D irradianceLUT;
layout(set = 3, binding = 2) uniform sampler2D prefilteredEnvLUT;
layout(set = 3, binding = 3) uniform sampler2D brdfLUT;

const float PI = 3.14159265359;
const float GROUND_RADIUS_KM = 6360.0;
const float ATMOSPHERE_RADIUS_KM = 6460.0;

// Exact screen-space Jacobian TBN for Vulkan
vec3 PerturbNormal(vec3 N, vec3 pos, vec2 uv) {
    vec3 mapN = texture(normalSampler, uv).xyz * 2.0 - 1.0;
    if (abs(mapN.x) < 0.01 && abs(mapN.y) < 0.01)
        return N;

    vec3 dp1 = dFdx(pos);
    vec3 dp2 = dFdy(pos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    float det = duv1.x * duv2.y - duv1.y * duv2.x;
    if (abs(det) < 1e-8)
        return N;

    float invDet = 1.0 / det;
    vec3 T = (dp1 * duv2.y - dp2 * duv1.y) * invDet;
    vec3 B = -((-dp1 * duv2.x + dp2 * duv1.x) * invDet);

    T = T - N * dot(N, T);
    float tLen = length(T);
    B = B - N * dot(N, B);
    float bLen = length(B);
    if (tLen < 1e-6 || bLen < 1e-6)
        return N;

    return normalize((T / tLen) * mapN.x + (B / bLen) * mapN.y + N * mapN.z);
}

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

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 EvaluatePBR(vec3 N, vec3 V, vec3 L, vec3 illuminanceLux, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 energyCompensation) {
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0)
        return vec3(0.0);

    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    float NdotV = max(dot(N, V), 0.001);
    vec3 specular = ((NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-5)) * energyCompensation;
    return (kD * albedo / PI + specular) * illuminanceLux * NdotL;
}

float ComputeSmoothDistanceAttenuation(float distSq, float radius) {
    float invDistSq = 1.0 / max(distSq, 0.0001);
    float invRadiusSq = 1.0 / max(radius * radius, 1e-4);
    float factor = distSq * invRadiusSq;
    float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
    return invDistSq * smoothFactor * smoothFactor;
}

void main() {
    vec4 textureColor = texture(texSampler, fragTexCoord);
    vec3 albedo = pow(textureColor.rgb, vec3(2.2)) * material.baseColorFactor.rgb * fragColor;
    float alpha = textureColor.a * material.baseColorFactor.a;

    vec4 mrSample = texture(metallicRoughnessSampler, fragTexCoord);
    float metallic = clamp(mrSample.b * material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * material.roughnessFactor, 0.04, 1.0);

    vec3 geomNorm = normalize(fragNormal);
    if (!gl_FrontFacing)
        geomNorm = -geomNorm;
    vec3 N = PerturbNormal(geomNorm, fragPosition, fragTexCoord);
    vec3 V = normalize(globalUniforms.cameraPosition - fragPosition);
    float NdotV = max(dot(N, V), 0.001);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec2 envBRDF = textureLod(brdfLUT, vec2(NdotV, roughness), 0.0).rg;
    vec3 energyCompensation = 1.0 + F0 * (1.0 / max(envBRDF.x + envBRDF.y, 1e-4) - 1.0);

    float camAltKm = max(globalUniforms.atmosphereAltitudeKm + max(globalUniforms.cameraPosition.y * 0.001, 0.0), 0.005);
    float camR = min(GROUND_RADIUS_KM + camAltKm, ATMOSPHERE_RADIUS_KM - 0.05);

    vec3 LoNits = vec3(0.0);
    vec3 sunDir = normalize(-globalUniforms.lightDirection);
    float sunHorizonFade = smoothstep(-0.04, 0.02, sunDir.y);
    vec3 sunTrans = textureLod(transmittanceLUT, TransmittanceUV(camR, sunDir.y), 0.0).rgb * sunHorizonFade;
    LoNits += EvaluatePBR(N, V, sunDir, globalUniforms.lightColor * globalUniforms.sunIlluminanceLux * sunTrans, albedo, metallic, roughness, F0, energyCompensation);

    for (uint i = 0u; i < globalUniforms.pointLightsCount; ++i) {
        PointLightData light = pointLights.lights[i];
        vec3 toLight = light.position - fragPosition;
        float distSq = dot(toLight, toLight);
        if (distSq >= light.radius * light.radius)
            continue;

        float dist = sqrt(max(distSq, 1e-8));
        vec3 L = toLight / dist;
        float luminousIntensityCd = light.luminousPowerLm / (4.0 * PI);
        float illuminanceLux = luminousIntensityCd * ComputeSmoothDistanceAttenuation(distSq, light.radius);
        LoNits += EvaluatePBR(N, V, L, light.color * illuminanceLux, albedo, metallic, roughness, F0, energyCompensation);
    }

    // Irradiance Lighting (Split-Sum IBL)
    vec3 F_ibl = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
    vec3 irradianceLux = textureLod(irradianceLUT, DirectionToEquirectUV(N), 0.0).rgb * globalUniforms.irradianceIntensity;
    vec3 diffuseIndirect = kD_ibl * (albedo / PI) * irradianceLux;
    vec3 prefilteredNits = SamplePrefilteredEnv(reflect(-V, N), roughness) * globalUniforms.irradianceIntensity;
    float roughGrazingAtten = mix(1.0 - roughness * roughness, 1.0, metallic);
    vec3 specularIndirect = prefilteredNits * (F0 * envBRDF.x + envBRDF.y * roughGrazingAtten) * energyCompensation;

    vec3 preExposedHdr = clamp((diffuseIndirect + specularIndirect + LoNits) * globalUniforms.cameraExposure, vec3(0.0), vec3(65000.0));
    outColor = vec4(preExposedHdr, alpha);
}
