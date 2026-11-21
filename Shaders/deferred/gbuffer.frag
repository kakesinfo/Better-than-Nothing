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
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inWorldNormal;
layout(location = 4) in vec4 inCurrClipPos;
layout(location = 5) in vec4 inPrevClipPos;

layout(location = 0) out vec4 outAlbedo;   // R8G8B8A8_UNORM: linear albedo.rgb, ao/valid
layout(location = 1) out vec2 outNormal;   // R16G16_SFLOAT: octahedral encoded world normal
layout(location = 2) out vec4 outMaterial; // R8G8B8A8_UNORM: metallic, roughness, ao, shadingModel
layout(location = 3) out vec2 outMotion;   // R16G16_SFLOAT: screen-space velocity (currUV - prevUV)

// Exact screen-space Jacobian TBN for Vulkan (handles top-down dFdy, arbitrary rotations, and mirrored UVs)
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
    vec3 B = -((-dp1 * duv2.x + dp2 * duv1.x) * invDet); // +Y in normal map points along -v (up)

    // Gram-Schmidt orthogonalization against shading normal
    T = T - N * dot(N, T);
    float tLen = length(T);
    B = B - N * dot(N, B);
    float bLen = length(B);
    if (tLen < 1e-6 || bLen < 1e-6)
        return N;

    T /= tLen;
    B /= bLen;
    return normalize(T * mapN.x + B * mapN.y + N * mapN.z);
}

vec2 OctWrap(vec2 v) {
    vec2 w = 1.0 - abs(v.yx);
    if (v.x < 0.0) w.x = -w.x;
    if (v.y < 0.0) w.y = -w.y;
    return w;
}

vec2 EncodeOctahedral(vec3 n) {
    n /= (abs(n.x) + abs(n.y) + abs(n.z) + 1e-8);
    if (n.z < 0.0)
        n.xy = OctWrap(n.xy);
    return n.xy;
}

void main() {
    vec4 texSample = texture(albedoSampler, inTexCoord);
    float alpha = texSample.a * material.baseColorFactor.a;
    if (material.alphaCutoff > 0.0 && alpha < material.alphaCutoff)
        discard;

    // Convert sRGB diffuse texture to linear space
    vec3 linearAlbedo = pow(texSample.rgb, vec3(2.2)) * material.baseColorFactor.rgb * inColor;

    vec3 geomN = normalize(inWorldNormal);
    if (!gl_FrontFacing)
        geomN = -geomN;
    vec3 N = PerturbNormal(geomN, inWorldPos, inTexCoord);

    // glTF 2.0 metallic-roughness convention: G = roughness, B = metallic
    vec4 mrSample = texture(metallicRoughnessSampler, inTexCoord);
    float metallic = clamp(mrSample.b * material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * material.roughnessFactor, 0.04, 1.0);

    vec2 currNDC = inCurrClipPos.xy / inCurrClipPos.w;
    vec2 prevNDC = inPrevClipPos.xy / inPrevClipPos.w;
    vec2 currUV = currNDC * 0.5 + 0.5;
    vec2 prevUV = prevNDC * 0.5 + 0.5;

    outAlbedo = vec4(clamp(linearAlbedo, 0.0, 1.0), 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(metallic, roughness, 1.0, 1.0);
    outMotion = currUV - prevUV;
}
