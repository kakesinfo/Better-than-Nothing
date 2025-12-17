#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 proj;
    mat4 view;
    vec3 lightPosition;
    vec3 cameraPosition;
} globalUniforms;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;


// Based on:
// - https://learnopengl.com/Lighting/Basic-Lighting
// - https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
    vec4 textureColor = texture(texSampler, fragTexCoord);

    if (textureColor.a < 0.5) {
        discard;
    }

    const vec3 lightColor = vec3(1.0, 1.0, 1.0);

    const float ambientStrength = 0.1;
    const float specularStrength = 0.75;

    const float kPi = 3.14159265;
    const float kShininess = 16.0;

    const vec3 norm = normalize(fragNormal);

    const vec3 lightDir = normalize(globalUniforms.lightPosition - fragPosition);
    const vec3 viewDir = normalize(globalUniforms.cameraPosition - fragPosition);
    const vec3 halfwayDir = normalize(lightDir + viewDir);

    const vec3 ambient = ambientStrength * lightColor;

    const float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Energy conservation: https://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
    // This normalization factor ensures that the total energy reflected by the surface doesn't exceed the incoming energy
    const float kEnergyConservation = (8.0 + kShininess) / (8.0 * kPi);
    const float spec = kEnergyConservation * pow(max(dot(norm, halfwayDir), 0.0), kShininess);
    vec3 specular = specularStrength * spec * lightColor;

    outColor = vec4(ambient + diffuse + specular, 1.0) * textureColor;
}
