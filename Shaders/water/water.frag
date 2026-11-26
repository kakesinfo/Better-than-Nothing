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
    float v0 = (b0 + clamp(eqUv.y, 0.01, 0.99)) / 5.0;
    return textureLod(prefilteredEnvLUT, vec2(eqUv.x, v0), 0.0).rgb;
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
    vec3 N = normalize(inWorldNormal);
    vec3 V = normalize(globalUniforms.cameraPosition - inWorldPos);
    vec3 L = normalize(-globalUniforms.lightDirection);
    vec3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    // Screen-space refraction
    vec2 refractUv = clamp(screenUv + N.xz * 0.025, vec2(0.001), vec2(0.999));
    float bgDepth = textureLod(sceneDepthTex, refractUv, 0.0).r;
    if (bgDepth < gl_FragCoord.z)
        refractUv = screenUv;

    vec3 bgColor = textureLod(sceneColorTex, refractUv, 0.0).rgb;

    // Beer-Lambert water absorption
    float waterDepth = max(inShoreDepth, 0.05);
    vec3 absorption = exp(-vec3(0.45, 0.16, 0.10) * waterDepth * 1.4);
    vec3 refracted = bgColor * absorption;

    // Fresnel (F0 = 0.02 for water)
    float fresnel = 0.02 + (1.0 - 0.02) * pow(1.0 - NdotV, 5.0);

    // Sky IBL reflection + GGX solar specular
    vec3 R = reflect(-V, N);
    vec3 envReflection = SamplePrefilteredSky(R, 0.06) * globalUniforms.cameraExposure;

    float D = DistributionGGX(N, H, 0.05);
    float spec = (D * fresnel) / max(4.0 * NdotV * NdotL + 0.001, 1e-4);
    vec3 sunSpec = globalUniforms.lightColor * globalUniforms.lightIntensity * spec * NdotL * globalUniforms.cameraExposure;

    // Shoreline foam
    float foamWave = sin(inWorldPos.x * 1.8 + inWorldPos.z * 1.5 - globalUniforms.elapsedTime * 2.5) * 0.5 + 0.5;
    float foamMask = (1.0 - smoothstep(0.02, 0.32, inShoreDepth)) * smoothstep(0.35, 0.85, foamWave);
    vec3 foamColor = (textureLod(irradianceLUT, DirToEquirectUV(N), 0.0).rgb + globalUniforms.lightColor * globalUniforms.lightIntensity * NdotL * 0.3) * globalUniforms.cameraExposure;

    vec3 waterColor = mix(refracted, envReflection, fresnel) + sunSpec;
    waterColor = mix(waterColor, foamColor, clamp(foamMask * 0.75, 0.0, 1.0));

    float alpha = clamp(smoothstep(0.0, 0.12, inShoreDepth), 0.0, 1.0);
    outHdrColor = vec4(waterColor, alpha);
}
