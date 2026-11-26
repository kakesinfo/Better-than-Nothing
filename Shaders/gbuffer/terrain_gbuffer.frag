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

layout(set = 1, binding = 0) uniform sampler2D albedoTex0;
layout(set = 1, binding = 1) uniform sampler2D normalTex0;
layout(set = 1, binding = 2) uniform sampler2D roughTex0;

layout(set = 1, binding = 3) uniform sampler2D albedoTex1;
layout(set = 1, binding = 4) uniform sampler2D normalTex1;
layout(set = 1, binding = 5) uniform sampler2D roughTex1;

layout(set = 1, binding = 6) uniform sampler2D albedoTex2;
layout(set = 1, binding = 7) uniform sampler2D normalTex2;
layout(set = 1, binding = 8) uniform sampler2D roughTex2;

layout(set = 1, binding = 9) uniform sampler2D albedoTex3;
layout(set = 1, binding = 10) uniform sampler2D normalTex3;
layout(set = 1, binding = 11) uniform sampler2D roughTex3;

layout(set = 1, binding = 12) uniform sampler2D albedoTex4;
layout(set = 1, binding = 13) uniform sampler2D normalTex4;
layout(set = 1, binding = 14) uniform sampler2D roughTex4;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec4 inCurrClipPos;
layout(location = 5) in vec4 inPrevClipPos;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outMaterial;
layout(location = 3) out vec2 outMotion;

vec2 OctWrap(vec2 v)
{
    vec2 s = vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
    return (1.0 - abs(v.yx)) * s;
}

vec2 EncodeOctahedral(vec3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z) + 1e-8);
    if (n.z < 0.0)
        n.xy = OctWrap(n.xy);
    return n.xy;
}

vec3 PerturbNormalWithMap(vec3 N, vec3 pos, vec2 uv, vec3 mapN)
{
    vec3 dp1 = dFdx(pos);
    vec3 dp2 = dFdy(pos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invMax = inversesqrt(max(dot(T, T), dot(B, B)) + 1e-8);
    T *= invMax;
    B *= invMax;

    T = normalize(T - N * dot(N, T));
    B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * mapN);
}

void SampleLayer(
    sampler2D albTex,
    sampler2D nrmTex,
    sampler2D rghTex,
    vec2 uv,
    vec2 macroUv,
    float macroBlend,
    out vec3 albedo,
    out vec3 normalSample,
    out float roughness)
{
    vec3 a0 = texture(albTex, uv).rgb;
    vec3 n0 = texture(nrmTex, uv).xyz * 2.0 - 1.0;
    float r0 = texture(rghTex, uv).r;

    if (macroBlend > 0.001)
    {
        vec3 a1 = texture(albTex, macroUv).rgb;
        vec3 n1 = texture(nrmTex, macroUv).xyz * 2.0 - 1.0;
        float r1 = texture(rghTex, macroUv).r;
        a0 = mix(a0, a1, macroBlend);
        n0 = normalize(mix(n0, n1, macroBlend));
        r0 = mix(r0, r1, macroBlend);
    }

    albedo = a0;
    normalSample = n0;
    roughness = r0;
}

void main()
{
    float distToCam = length(globalUniforms.cameraPosition - inWorldPos);
    float macroBlend = smoothstep(26.0, 64.0, distToCam) * 0.6;
    vec2 macroUv = mat2(0.866, -0.5, 0.5, 0.866) * (inTexCoord * 0.27) + vec2(0.37, 0.71);

    float wGrass = inColor.x;
    float wRock = inColor.y;
    float wMud = inColor.z;
    float wSand = smoothstep(-1.10, -1.70, inWorldPos.y);
    float wDirt = max(0.0, 1.0 - min(1.0, wGrass + wRock + wMud + wSand));
    float wSum = max(wDirt + wGrass + wRock + wMud + wSand, 1e-4);

    wDirt /= wSum;
    wGrass /= wSum;
    wRock /= wSum;
    wMud /= wSum;
    wSand /= wSum;

    vec3 a0, n0; float r0;
    vec3 a1, n1; float r1;
    vec3 a2, n2; float r2;
    vec3 a3, n3; float r3;
    vec3 a4, n4; float r4;

    SampleLayer(albedoTex0, normalTex0, roughTex0, inTexCoord, macroUv, macroBlend, a0, n0, r0);
    SampleLayer(albedoTex1, normalTex1, roughTex1, inTexCoord, macroUv, macroBlend, a1, n1, r1);
    SampleLayer(albedoTex2, normalTex2, roughTex2, inTexCoord, macroUv, macroBlend, a2, n2, r2);
    SampleLayer(albedoTex3, normalTex3, roughTex3, inTexCoord, macroUv, macroBlend, a3, n3, r3);
    SampleLayer(albedoTex4, normalTex4, roughTex4, inTexCoord, macroUv, macroBlend, a4, n4, r4);

    vec3 blendedAlbedo = a0 * wDirt + a1 * wGrass + a2 * wRock + a3 * wMud + a4 * wSand;
    vec3 blendedMapN = normalize(n0 * wDirt + n1 * wGrass + n2 * wRock + n3 * wMud + n4 * wSand);
    float blendedRough = clamp(r0 * wDirt + r1 * wGrass + r2 * wRock + r3 * wMud + r4 * wSand, 0.55, 0.98);

    vec3 baseN = normalize(inWorldNormal);
    vec3 N = PerturbNormalWithMap(baseN, inWorldPos, inTexCoord, blendedMapN);

    vec2 currNdc = inCurrClipPos.xy / max(inCurrClipPos.w, 1e-6);
    vec2 prevNdc = inPrevClipPos.xy / max(inPrevClipPos.w, 1e-6);

    outAlbedo = vec4(blendedAlbedo, 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(0.0, blendedRough, 1.0, 0.0);
    outMotion = (currNdc - prevNdc) * 0.5;
}
