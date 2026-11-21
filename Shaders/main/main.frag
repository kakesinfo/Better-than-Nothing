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

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

layout(std430, set = 2, binding = 0) readonly buffer PointLightBuffer {
    PointLightData lights[];
} pointLights;

const float ambientStrength = 0.1;
const float specularStrength = 0.3;

// Screen-space cotangent frame (Christian Schuler, "Followup: Normal Mapping Without Precomputed Tangents")
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
    // Fast path for default flat normal (0, 0, 1)
    if (abs(mapN.x) < 0.01 && abs(mapN.y) < 0.01)
        return N;
    mat3 TBN = ComputeCotangentFrame(N, pos, uv);
    return normalize(TBN * mapN);
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

    vec4 textureColor = texture(texSampler, fragTexCoord);
    textureColor.rgb = pow(textureColor.rgb, vec3(gammaCorrection));

    const vec3 geomNorm = normalize(fragNormal);
    const vec3 norm = PerturbNormal(geomNorm, fragPosition, fragTexCoord);
    const vec3 viewDir = normalize(globalUniforms.cameraPosition - fragPosition);

    vec3 resultColor = CalculateDirectionalLight(norm, viewDir);
    for (int i = 0; i < globalUniforms.pointLightsCount; i++)
        resultColor += CalculatePointLight(pointLights.lights[i], norm, viewDir);

    const vec3 hdrColor = resultColor * textureColor.rgb;

    const vec3 exposedColor = hdrColor * exposure;
    const vec3 mappedColor = exposedColor / (exposedColor + vec3(1.0)); // TONE MAPPING (Reinhard)
    const vec3 correctedColor = pow(mappedColor, vec3(1.0 / gammaCorrection)); // GAMMA CORRECTION (power 2.2)

    outColor = vec4(correctedColor, textureColor.a);
}
