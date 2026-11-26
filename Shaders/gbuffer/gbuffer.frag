#version 450

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(set = 1, binding = 3) uniform MaterialUniformBuffer {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    float _pad;
} material;

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

vec3 PerturbNormal(vec3 N, vec3 pos, vec2 uv)
{
    vec3 mapN = texture(normalSampler, uv).xyz * 2.0 - 1.0;
    if (abs(mapN.x) < 0.01 && abs(mapN.y) < 0.01)
        return N;

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

    // Gram-Schmidt orthogonalization
    T = normalize(T - N * dot(N, T));
    B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * mapN);
}

void main()
{
    vec4 albedoSample = texture(albedoSampler, inTexCoord);
    float alpha = albedoSample.a * material.baseColorFactor.a;
    if (material.alphaCutoff > 0.0 && alpha < material.alphaCutoff)
        discard;

    vec3 linearAlbedo = pow(albedoSample.rgb, vec3(2.2)) * material.baseColorFactor.rgb * inColor;
    vec3 geomN = normalize(inWorldNormal);
    if (!gl_FrontFacing)
        geomN = -geomN;
    vec3 N = PerturbNormal(geomN, inWorldPos, inTexCoord);

    vec4 mrSample = texture(metallicRoughnessSampler, inTexCoord);
    float metallic = clamp(mrSample.b * material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * material.roughnessFactor, 0.04, 1.0);
    float ao = mrSample.r;

    vec2 currNDC = inCurrClipPos.xy / max(abs(inCurrClipPos.w), 1e-6);
    vec2 prevNDC = inPrevClipPos.xy / max(abs(inPrevClipPos.w), 1e-6);
    vec2 motion = (currNDC - prevNDC) * 0.5;

    outAlbedo = vec4(linearAlbedo, 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(metallic, roughness, ao, 1.0);
    outMotion = motion;
}
