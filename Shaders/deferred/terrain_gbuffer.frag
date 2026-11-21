#version 450

// Multi-Layer Terrain PBR G-Buffer Fragment Shader
// Blends 5 Ground PBR texture packs (Color + NormalGL + Roughness):
//   Layer 0: Ground049A (Sand / Riverbed / Shore)
//   Layer 1: Ground068  (Mossy Forest Grass)
//   Layer 2: Ground071  (Forest Dirt / Twigs)
//   Layer 3: Ground086  (Mossy Rock / Hillsides)
//   Layer 4: Ground108  (Rocky Gravel / Steep Cliffs)

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

    float time;
    float waterLevel;
    float waterWaveScale;
    float waterFoamIntensity;
} globalUniforms;

layout(set = 1, binding = 0)  uniform sampler2D sandAlbedo;
layout(set = 1, binding = 1)  uniform sampler2D sandNormal;
layout(set = 1, binding = 2)  uniform sampler2D sandRoughness;

layout(set = 1, binding = 3)  uniform sampler2D grassAlbedo;
layout(set = 1, binding = 4)  uniform sampler2D grassNormal;
layout(set = 1, binding = 5)  uniform sampler2D grassRoughness;

layout(set = 1, binding = 6)  uniform sampler2D dirtAlbedo;
layout(set = 1, binding = 7)  uniform sampler2D dirtNormal;
layout(set = 1, binding = 8)  uniform sampler2D dirtRoughness;

layout(set = 1, binding = 9)  uniform sampler2D mossyRockAlbedo;
layout(set = 1, binding = 10) uniform sampler2D mossyRockNormal;
layout(set = 1, binding = 11) uniform sampler2D mossyRockRoughness;

layout(set = 1, binding = 12) uniform sampler2D gravelAlbedo;
layout(set = 1, binding = 13) uniform sampler2D gravelNormal;
layout(set = 1, binding = 14) uniform sampler2D gravelRoughness;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inColor; // (wSand, wGrass, wDirt); wRock = max(0, 1 - r - g - b)
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inWorldNormal;
layout(location = 4) in vec4 inCurrClipPos;
layout(location = 5) in vec4 inPrevClipPos;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outMaterial;
layout(location = 3) out vec2 outMotion;

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

float Hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float ValueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Dual-scale anti-tiling sample: blends near detail UV with rotated macro UV
vec3 SampleAntiTileAlbedo(sampler2D tex, vec2 uv, vec2 macroUV, float macroBlend) {
    vec3 c0 = pow(texture(tex, uv).rgb, vec3(2.2));
    vec3 c1 = pow(texture(tex, macroUV).rgb, vec3(2.2));
    return mix(c0, c1, macroBlend);
}

vec3 SampleAntiTileNormalTS(sampler2D tex, vec2 uv, vec2 macroUV, float macroBlend) {
    vec3 n0 = texture(tex, uv).xyz * 2.0 - 1.0;
    vec3 n1 = texture(tex, macroUV).xyz * 2.0 - 1.0;
    return normalize(mix(n0, n1, macroBlend * 0.65));
}

float SampleAntiTileRough(sampler2D tex, vec2 uv, vec2 macroUV, float macroBlend) {
    float r0 = texture(tex, uv).r;
    float r1 = texture(tex, macroUV).r;
    return mix(r0, r1, macroBlend);
}

// Vulkan Jacobian TBN perturbation using blended tangent-space normal
vec3 ApplyTangentNormal(vec3 N, vec3 pos, vec2 uv, vec3 mapN) {
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

    T /= tLen;
    B /= bLen;
    return normalize(T * mapN.x + B * mapN.y + N * mapN.z);
}

void main() {
    vec3 geomN = normalize(inWorldNormal);
    float slope = clamp(1.0 - geomN.y, 0.0, 1.0);

    // Rotated macro UV breaks up visible repetition across large terrain chunks
    const mat2 rot = mat2(0.80, -0.60, 0.60, 0.80);
    vec2 uv = inTexCoord;
    vec2 macroUV = (rot * inTexCoord) * 0.27 + vec2(3.17, 1.91);
    float macroBlend = 0.32 + 0.16 * ValueNoise(inWorldPos.xz * 0.045);

    // Splat weights from vertex color + split rock weight between mossy rock (Ground086) and gravel scree (Ground108)
    float wSand = clamp(inColor.r, 0.0, 1.0);
    float wGrass = clamp(inColor.g, 0.0, 1.0);
    float wDirt = clamp(inColor.b, 0.0, 1.0);
    float wRockTotal = max(0.0, 1.0 - wSand - wGrass - wDirt);

    float rockSplitNoise = ValueNoise(inWorldPos.xz * 0.08 + vec2(7.3, 19.1));
    float gravelRatio = smoothstep(0.20, 0.46, slope + (rockSplitNoise - 0.5) * 0.22);
    float wMossyRock = wRockTotal * (1.0 - gravelRatio);
    float wGravel = wRockTotal * gravelRatio;

    // Subtle height/noise modulation to sharpen natural biome transitions
    float microBreakup = (ValueNoise(inWorldPos.xz * 0.35) - 0.5) * 0.14;
    wSand = max(wSand + microBreakup * wSand, 0.0);
    wGrass = max(wGrass - microBreakup * wGrass, 0.0);
    wDirt = max(wDirt + microBreakup * 0.5 * wDirt, 0.0);

    float totalW = max(wSand + wGrass + wDirt + wMossyRock + wGravel, 1e-5);
    wSand /= totalW;
    wGrass /= totalW;
    wDirt /= totalW;
    wMossyRock /= totalW;
    wGravel /= totalW;

    vec3 albedo = vec3(0.0);
    vec3 normalTS = vec3(0.0, 0.0, 0.001);
    float roughness = 0.0;

    if (wSand > 0.005) {
        albedo += wSand * SampleAntiTileAlbedo(sandAlbedo, uv, macroUV, macroBlend);
        normalTS += wSand * SampleAntiTileNormalTS(sandNormal, uv, macroUV, macroBlend);
        roughness += wSand * SampleAntiTileRough(sandRoughness, uv, macroUV, macroBlend);
    }
    if (wGrass > 0.005) {
        albedo += wGrass * SampleAntiTileAlbedo(grassAlbedo, uv, macroUV, macroBlend);
        normalTS += wGrass * SampleAntiTileNormalTS(grassNormal, uv, macroUV, macroBlend);
        roughness += wGrass * SampleAntiTileRough(grassRoughness, uv, macroUV, macroBlend);
    }
    if (wDirt > 0.005) {
        albedo += wDirt * SampleAntiTileAlbedo(dirtAlbedo, uv, macroUV, macroBlend);
        normalTS += wDirt * SampleAntiTileNormalTS(dirtNormal, uv, macroUV, macroBlend);
        roughness += wDirt * SampleAntiTileRough(dirtRoughness, uv, macroUV, macroBlend);
    }
    if (wMossyRock > 0.005) {
        albedo += wMossyRock * SampleAntiTileAlbedo(mossyRockAlbedo, uv * 0.75, macroUV * 0.85, macroBlend);
        normalTS += wMossyRock * SampleAntiTileNormalTS(mossyRockNormal, uv * 0.75, macroUV * 0.85, macroBlend);
        roughness += wMossyRock * SampleAntiTileRough(mossyRockRoughness, uv * 0.75, macroUV * 0.85, macroBlend);
    }
    if (wGravel > 0.005) {
        albedo += wGravel * SampleAntiTileAlbedo(gravelAlbedo, uv * 0.9, macroUV, macroBlend);
        normalTS += wGravel * SampleAntiTileNormalTS(gravelNormal, uv * 0.9, macroUV, macroBlend);
        roughness += wGravel * SampleAntiTileRough(gravelRoughness, uv * 0.9, macroUV, macroBlend);
    }

    // Subtle macro world-space color variation so distant hillsides look organic
    float macroVar = ValueNoise(inWorldPos.xz * 0.025);
    albedo *= mix(0.88, 1.12, macroVar);

    // Wet darkening near/under water line
    float wetness = 1.0 - smoothstep(-0.2, 0.55, inWorldPos.y - globalUniforms.waterLevel);
    albedo *= mix(1.0, 0.58, wetness);
    roughness = mix(clamp(roughness, 0.25, 0.98), 0.18, wetness * 0.75);

    vec3 N = ApplyTangentNormal(geomN, inWorldPos, uv, normalize(normalTS));

    vec2 currNDC = inCurrClipPos.xy / inCurrClipPos.w;
    vec2 prevNDC = inPrevClipPos.xy / inPrevClipPos.w;
    vec2 currUV = currNDC * 0.5 + 0.5;
    vec2 prevUV = prevNDC * 0.5 + 0.5;

    outAlbedo = vec4(clamp(albedo, 0.0, 1.0), 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(0.0, roughness, 1.0, 1.0);
    outMotion = currUV - prevUV;
}
