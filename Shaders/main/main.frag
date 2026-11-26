#version 450

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

const float PI = 3.14159265359;

mat3 ComputeCotangentFrame(vec3 N, vec3 p, vec2 uv)
{
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invMax = inversesqrt(max(dot(T, T), dot(B, B)) + 1e-8);
    return mat3(T * invMax, B * invMax, N);
}

vec3 PerturbNormal(vec3 N, vec3 pos, vec2 uv)
{
    vec3 mapN = texture(normalSampler, uv).xyz * 2.0 - 1.0;
    if (abs(mapN.x) < 0.01 && abs(mapN.y) < 0.01)
        return N;
    mat3 TBN = ComputeCotangentFrame(N, pos, uv);
    return normalize(TBN * mapN);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 EvaluatePBR(vec3 albedo, vec3 N, vec3 V, vec3 L, float metallic, float roughness)
{
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);

    return (kD * albedo / PI + specular) * NdotL;
}

void main()
{
    vec4 texColor = texture(texSampler, fragTexCoord);
    float alpha = texColor.a * material.baseColorFactor.a;
    vec3 albedo = pow(texColor.rgb, vec3(2.2)) * material.baseColorFactor.rgb * fragColor;

    vec4 mrSample = texture(metallicRoughnessSampler, fragTexCoord);
    float metallic = clamp(mrSample.b * material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * material.roughnessFactor, 0.04, 1.0);
    float ao = mrSample.r;

    vec3 geomNorm = normalize(fragNormal);
    vec3 N = PerturbNormal(geomNorm, fragPosition, fragTexCoord);
    vec3 V = normalize(globalUniforms.cameraPosition - fragPosition);

    vec3 sunL = normalize(-globalUniforms.lightDirection);
    vec3 sunRadiance = globalUniforms.lightColor * globalUniforms.lightIntensity;
    vec3 Lo = EvaluatePBR(albedo, N, V, sunL, metallic, roughness) * sunRadiance;

    for (uint i = 0u; i < globalUniforms.pointLightCount; ++i)
    {
        PointLightData light = pointLights.lights[i];
        vec3 toLight = light.position - fragPosition;
        float dist = length(toLight);
        if (dist > light.radius || dist < 1e-5)
            continue;

        vec3 L = toLight / dist;
        float distRatio = dist / max(light.radius, 1e-4);
        float window = pow(clamp(1.0 - pow(distRatio, 4.0), 0.0, 1.0), 2.0);
        float attenuation = window / (dist * dist + 1.0);
        vec3 radiance = light.color * (light.luminousPowerLm / (4.0 * PI)) * attenuation;

        Lo += EvaluatePBR(albedo, N, V, L, metallic, roughness) * radiance;
    }

    vec3 ambient = albedo * (1.0 - metallic) * 0.03 * globalUniforms.lightIntensity * ao;
    vec3 hdrLuminance = ambient + Lo;
    vec3 preExposedHdr = hdrLuminance * globalUniforms.cameraExposure;

    outColor = vec4(preExposedHdr, alpha);
}
