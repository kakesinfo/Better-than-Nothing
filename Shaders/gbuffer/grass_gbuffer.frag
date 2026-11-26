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

void main()
{
    float heightFactor = clamp(inTexCoord.y, 0.0, 1.0);
    float tint = inColor.y;

    vec3 rootColor = vec3(0.035, 0.085, 0.018);
    vec3 tipColor = vec3(0.16, 0.29, 0.055) * tint;
    vec3 albedo = mix(rootColor, tipColor, smoothstep(0.0, 1.0, heightFactor));

    vec3 N = normalize(inWorldNormal);
    if (!gl_FrontFacing)
        N = -N;

    vec2 currNdc = inCurrClipPos.xy / max(inCurrClipPos.w, 1e-6);
    vec2 prevNdc = inPrevClipPos.xy / max(inPrevClipPos.w, 1e-6);

    outAlbedo = vec4(albedo, 1.0);
    outNormal = EncodeOctahedral(N);
    outMaterial = vec4(0.0, 0.78, mix(0.45, 1.0, heightFactor), 0.0);
    outMotion = (currNdc - prevNdc) * 0.5;
}
