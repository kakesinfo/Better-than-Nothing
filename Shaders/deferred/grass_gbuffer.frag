#version 450

// Procedural 3D Grass Blade G-Buffer Fragment Shader
// - Root-to-tip botanical color gradient + per-blade & macro meadow hue variation
// - Rounded 3D blade spine normal + root ambient occlusion (stored in outMaterial.b)
// - Matte organic roughness so grass never looks plastic or wet

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inColor;    // r = hue variation, g = stiffness, b = random phase
layout(location = 2) in vec2 inTexCoord; // x = across blade [0..1], y = normalized height h [0..1]
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

void main() {
    float u = clamp(inTexCoord.x, 0.0, 1.0);
    float h = clamp(inTexCoord.y, 0.0, 1.0);
    float hueVar = inColor.r;

    // Botanical root-to-tip linear-space albedo palette (calibrated to match Grass002 meadow floor)
    vec3 rootCol = vec3(0.022, 0.045, 0.012);
    vec3 midColCool = vec3(0.052, 0.115, 0.024);
    vec3 midColWarm = vec3(0.078, 0.132, 0.026);
    vec3 tipCol = vec3(0.125, 0.178, 0.042);

    float macroPatch = ValueNoise(inWorldPos.xz * 0.045);
    vec3 midCol = mix(midColCool, midColWarm, clamp(hueVar * 0.65 + macroPatch * 0.35, 0.0, 1.0));

    vec3 bladeAlbedo = mix(rootCol, midCol, smoothstep(0.0, 0.55, h));
    bladeAlbedo = mix(bladeAlbedo, tipCol, smoothstep(0.50, 1.0, h));

    // Subtle central vein highlight along the spine of the blade
    float spine = 1.0 - abs(u * 2.0 - 1.0);
    bladeAlbedo *= mix(0.92, 1.08, smoothstep(0.55, 1.0, spine));

    // Orient normal: blend blade face normal with terrain up-vector so grass integrates smoothly with ground lighting
    vec3 faceN = normalize(inWorldNormal);
    if (!gl_FrontFacing)
        faceN = -faceN;
    vec3 N = normalize(mix(vec3(0.0, 1.0, 0.0), faceN, 0.42));

    // Self-shadowing ambient occlusion at the dense base of the grass clump
    float bladeAO = mix(0.30, 1.0, pow(h, 0.65));

    // Matte organic roughness (0.94 at base -> 0.84 at tip)
    float roughness = mix(0.94, 0.84, h);

    vec2 currNDC = inCurrClipPos.xy / inCurrClipPos.w;
    vec2 prevNDC = inPrevClipPos.xy / inPrevClipPos.w;
    vec2 currUV = currNDC * 0.5 + 0.5;
    vec2 prevUV = prevNDC * 0.5 + 0.5;

    outAlbedo = vec4(clamp(bladeAlbedo, 0.0, 1.0), 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(0.0, roughness, bladeAO, 1.0);
    outMotion = currUV - prevUV;
}
