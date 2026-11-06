#version 450

struct PointLightData {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    float intensity;
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
    vec3 lightDirection;
    vec3 lightColor;
    float lightIntensity;
    vec3 cameraPosition;
    float cameraExposure;
    int pointLightsCount;
} globalUniforms;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 3) uniform sampler2D occlusionMap;
layout(set = 1, binding = 4) uniform sampler2D emissiveMap;
layout(set = 1, binding = 5) uniform MaterialUniforms {
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    uint flags;
} material;

layout(std430, set = 2, binding = 0) readonly buffer PointLightBuffer {
    PointLightData lights[];
} pointLights;

const float ambientStrength = 0.1;
const float specularStrength = 0.3;

vec3 CalculateNormal()
{
    vec3 normal = normalize(fragNormal);
    vec3 tangent = normalize(fragTangent.xyz);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent) * fragTangent.w;
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3 normalSample = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0;
    return normalize(TBN * normalSample);
}

float CalculateEnergyConservation(vec3 norm, vec3 halfwayDir)
{
    // Energy conservation: https://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
    // This normalization factor ensures that the total energy reflected by the surface doesn't exceed the incoming energy

    const float kPi = 3.14159265;
    const float kShininess = 16.0;

    const float kEnergyConservation = (8.0 + kShininess) / (8.0 * kPi);
    const float spec = kEnergyConservation * pow(max(dot(norm, halfwayDir), 0.0), kShininess);

    return spec;
}

vec3 CalculateDirectionalLight(vec3 norm, vec3 viewDir)
{
    const vec3 lightDir = normalize(-globalUniforms.lightDirection);
    const vec3 halfwayDir = normalize(lightDir + viewDir);

    const float diff = max(dot(norm, lightDir), 0.0);
    const float spec = CalculateEnergyConservation(norm, halfwayDir);

    const float intensity = globalUniforms.lightIntensity;
    const vec3 ambient = ambientStrength * globalUniforms.lightColor * intensity;
    const vec3 diffuse = diff * globalUniforms.lightColor * intensity;
    const vec3 specular = specularStrength * spec * globalUniforms.lightColor * intensity;

    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLightData light, vec3 norm, vec3 viewDir)
{
    const vec3 lightDir = normalize(light.position - fragPosition);
    const vec3 halfwayDir = normalize(lightDir + viewDir);

    const float diff = max(dot(norm, lightDir), 0.0);
    const float spec = CalculateEnergyConservation(norm, halfwayDir);

    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    const vec3 diffuse = diff * light.color * light.intensity * attenuation;
    const vec3 specular = spec * light.color * light.intensity * attenuation;

    return (diffuse + specular);
}

// Based on:
// - https://learnopengl.com/Lighting/Basic-Lighting
// - https://learnopengl.com/Lighting/Multiple-lights
// - https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
// - https://learnopengl.com/Advanced-Lighting/HDR
void main() {
    const float exposure = globalUniforms.cameraExposure;
    const float gammaCorrection = 2.2;

    vec4 albedoSample = texture(albedoMap, fragTexCoord) * material.baseColorFactor;
    if (albedoSample.a < material.alphaCutoff)
        discard;

    albedoSample.rgb = pow(albedoSample.rgb, vec3(gammaCorrection));

    const vec3 norm = CalculateNormal();
    const vec3 viewDir = normalize(globalUniforms.cameraPosition - fragPosition);

    vec3 resultColor = CalculateDirectionalLight(norm, viewDir);
    for (int i = 0; i < globalUniforms.pointLightsCount; i++)
        resultColor += CalculatePointLight(pointLights.lights[i], norm, viewDir);

    float ao = texture(occlusionMap, fragTexCoord).r;
    resultColor *= ao;

    vec3 emissive = texture(emissiveMap, fragTexCoord).rgb * material.emissiveFactor;
    emissive = pow(emissive, vec3(gammaCorrection));

    const vec3 hdrColor = (resultColor * albedoSample.rgb) + emissive;

    const vec3 exposedColor = hdrColor * exposure;
    const vec3 mappedColor = exposedColor / (exposedColor + vec3(1.0)); // TONE MAPPING (Reinhard)
    const vec3 correctedColor = pow(mappedColor, vec3(1.0 / gammaCorrection)); // GAMMA CORRECTION (power 2.2)

    outColor = vec4(correctedColor, albedoSample.a);
}
