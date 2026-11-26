#version 450

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
    vec2 w = 1.0 - abs(v.yx);
    if (v.x < 0.0) w.x = -w.x;
    if (v.y < 0.0) w.y = -w.y;
    return w;
}

vec2 EncodeOctahedral(vec3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z) + 1e-8);
    if (n.z < 0.0)
        n.xy = OctWrap(n.xy);
    return n.xy;
}

float Hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float ValueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main()
{
    float u = clamp(inTexCoord.x, 0.0, 1.0);
    float h = clamp(inTexCoord.y, 0.0, 1.0);
    float hueVar = clamp(inColor.y - 0.75, 0.0, 0.4) / 0.4;

    vec3 rootCol = vec3(0.022, 0.045, 0.012);
    vec3 midColCool = vec3(0.052, 0.115, 0.024);
    vec3 midColWarm = vec3(0.078, 0.132, 0.026);
    vec3 tipCol = vec3(0.125, 0.178, 0.042);

    float macroPatch = ValueNoise(inWorldPos.xz * 0.045);
    vec3 midCol = mix(midColCool, midColWarm, clamp(hueVar * 0.65 + macroPatch * 0.35, 0.0, 1.0));

    vec3 bladeAlbedo = mix(rootCol, midCol, smoothstep(0.0, 0.55, h));
    bladeAlbedo = mix(bladeAlbedo, tipCol, smoothstep(0.50, 1.0, h));

    float spine = 1.0 - abs(u * 2.0 - 1.0);
    bladeAlbedo *= mix(0.92, 1.08, smoothstep(0.55, 1.0, spine));

    vec3 faceN = normalize(inWorldNormal);
    if (!gl_FrontFacing)
        faceN = -faceN;
    vec3 N = normalize(mix(vec3(0.0, 1.0, 0.0), faceN, 0.42));

    float bladeAO = mix(0.30, 1.0, pow(h, 0.65));
    float roughness = mix(0.94, 0.84, h);

    vec2 currNdc = inCurrClipPos.xy / max(inCurrClipPos.w, 1e-6);
    vec2 prevNdc = inPrevClipPos.xy / max(inPrevClipPos.w, 1e-6);

    outAlbedo = vec4(clamp(bladeAlbedo, 0.0, 1.0), 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(0.0, roughness, bladeAO, 0.0);
    outMotion = (currNdc - prevNdc) * 0.5;
}
