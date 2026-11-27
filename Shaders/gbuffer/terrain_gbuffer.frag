#version 450

// Multi-Layer Terrain PBR G-Buffer Fragment Shader
// Blends 5 PBR texture packs (Color + NormalGL + Roughness):
//   Layer 0: Ground049A (Dry Sand / Riverbed / Shore)
//   Layer 1: Grass002   (Lush Meadow Grass)
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

// Sharp near detail; only blends rotated far UV at distance to eliminate tiling without blurring grass blades
vec3 SampleTerrainAlbedo(sampler2D tex, vec2 uv, vec2 farUV, float farBlend) {
    vec3 cNear = pow(texture(tex, uv).rgb, vec3(2.2));
    if (farBlend <= 0.005)
        return cNear;
    vec3 cFar = pow(texture(tex, farUV).rgb, vec3(2.2));
    return mix(cNear, cFar, farBlend);
}

vec3 SampleTerrainNormalTS(sampler2D tex, vec2 uv, float strength) {
    vec3 n = texture(tex, uv).xyz * 2.0 - 1.0;
    n.xy *= strength;
    n.z = max(n.z, 0.1);
    return normalize(n);
}

float SampleTerrainRoughness(sampler2D tex, vec2 uv, float minR, float maxR) {
    float r = texture(tex, uv).r;
    return mix(minR, maxR, r);
}

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
    float distToCam = length(globalUniforms.cameraPosition - inWorldPos);

    // Keep grass blades & sand grains 100% crisp up close; only blend rotated far UV beyond 28m
    const mat2 rot = mat2(0.80, -0.60, 0.60, 0.80);
    vec2 uv = inTexCoord;
    vec2 farUV = (rot * inTexCoord) * 0.35 + vec2(3.17, 1.91);
    float farBlend = smoothstep(26.0, 85.0, distToCam) * 0.55;
    float normalDistFade = 1.0 - smoothstep(35.0, 110.0, distToCam) * 0.65;

    float wSand = clamp(inColor.r, 0.0, 1.0);
    float wGrass = clamp(inColor.g, 0.0, 1.0);
    float wDirt = clamp(inColor.b, 0.0, 1.0);
    float wRockTotal = max(0.0, 1.0 - wSand - wGrass - wDirt);

    float rockSplitNoise = ValueNoise(inWorldPos.xz * 0.08 + vec2(7.3, 19.1));
    float gravelRatio = smoothstep(0.20, 0.46, slope + (rockSplitNoise - 0.5) * 0.22);
    float wMossyRock = wRockTotal * (1.0 - gravelRatio);
    float wGravel = wRockTotal * gravelRatio;

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
        albedo += wSand * SampleTerrainAlbedo(sandAlbedo, uv * 0.75, farUV * 0.75, farBlend);
        normalTS += wSand * SampleTerrainNormalTS(sandNormal, uv * 0.75, 0.45 * normalDistFade);
        roughness += wSand * SampleTerrainRoughness(sandRoughness, uv * 0.75, 0.85, 0.96);
    }
    if (wGrass > 0.005) {
        vec3 grassCol = SampleTerrainAlbedo(grassAlbedo, uv, farUV, farBlend);
        albedo += wGrass * grassCol;
        normalTS += wGrass * SampleTerrainNormalTS(grassNormal, uv, 0.50 * normalDistFade);
        roughness += wGrass * SampleTerrainRoughness(grassRoughness, uv, 0.88, 0.98);
    }
    if (wDirt > 0.005) {
        albedo += wDirt * SampleTerrainAlbedo(dirtAlbedo, uv * 0.85, farUV * 0.85, farBlend);
        normalTS += wDirt * SampleTerrainNormalTS(dirtNormal, uv * 0.85, 0.50 * normalDistFade);
        roughness += wDirt * SampleTerrainRoughness(dirtRoughness, uv * 0.85, 0.88, 0.98);
    }
    if (wMossyRock > 0.005) {
        albedo += wMossyRock * SampleTerrainAlbedo(mossyRockAlbedo, uv * 0.55, farUV * 0.65, farBlend);
        normalTS += wMossyRock * SampleTerrainNormalTS(mossyRockNormal, uv * 0.55, 0.70 * normalDistFade);
        roughness += wMossyRock * SampleTerrainRoughness(mossyRockRoughness, uv * 0.55, 0.78, 0.94);
    }
    if (wGravel > 0.005) {
        albedo += wGravel * SampleTerrainAlbedo(gravelAlbedo, uv * 0.70, farUV * 0.70, farBlend);
        normalTS += wGravel * SampleTerrainNormalTS(gravelNormal, uv * 0.70, 0.65 * normalDistFade);
        roughness += wGravel * SampleTerrainRoughness(gravelRoughness, uv * 0.70, 0.82, 0.96);
    }

    // Gentle large-scale meadow/soil color variation so open plains feel natural
    float macroVar = ValueNoise(inWorldPos.xz * 0.03);
    albedo *= mix(0.90, 1.12, macroVar);

    // Subtle damp darkening ONLY right at the water edge (keeps sand matte, never glossy plastic)
    float dampEdge = 1.0 - smoothstep(-0.15, 0.18, inWorldPos.y - globalUniforms.waterLevel);
    albedo *= mix(1.0, 0.74, dampEdge);
    roughness = mix(clamp(roughness, 0.78, 0.99), 0.68, dampEdge * 0.5);

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
