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

    float irradianceIntensity;
    float waterWaveScale;
    float waterFoamIntensity;
    float grassWindStrength;

    float grassWindSpeed;
    float grassFadeDistance;
    float cloudDensity;
    float cloudShadowStrength;
} globalUniforms;

layout(set = 1, binding = 0) uniform sampler2D hdrTex;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

vec3 AgXDefaultContrastApprox(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return + 15.5     * x4 * x2
           - 40.14    * x4 * x
           + 31.96    * x4
           - 6.868    * x2 * x
           + 0.4298   * x2
           + 0.1191   * x
           - 0.00232;
}

vec3 ToneMapAgX(vec3 color)
{
    const mat3 agx_mat = mat3(
        0.842479062253094, 0.0423282422610123, 0.0423756549057051,
        0.0784335999999992, 0.878468636469772, 0.0784336,
        0.0792237451477643, 0.0791661274605434, 0.879142973793104
    );
    const mat3 agx_mat_inv = mat3(
        1.19687900512017, -0.0528968517574562, -0.0529716355144438,
        -0.0980208811401368, 1.15190312990417, -0.0980434501171241,
        -0.0990297440797205, -0.0989611768448433, 1.15107367264116
    );

    const float minEv = -12.47393;
    const float maxEv = 4.026069;

    vec3 val = agx_mat * max(color, vec3(0.0));
    val = clamp(log2(max(val, vec3(1e-6))), minEv, maxEv);
    val = (val - minEv) / (maxEv - minEv);
    val = AgXDefaultContrastApprox(val);

    // Subtle punchy look (matching test/full-pass)
    const vec3 lumaWeights = vec3(0.2126, 0.7152, 0.0722);
    float luma = dot(val, lumaWeights);
    val = pow(max(val, vec3(0.0)), vec3(1.15));
    val = vec3(luma) + 1.1 * (val - vec3(luma));

    return clamp(agx_mat_inv * val, 0.0, 1.0);
}

vec3 ToneMapACES(vec3 color)
{
    const mat3 acesInput = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
    );
    const mat3 acesOutput = mat3(
         1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
    );

    vec3 v = acesInput * max(color, vec3(0.0));
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return clamp(acesOutput * (a / b), 0.0, 1.0);
}

vec3 ToneMapReinhard(vec3 x)
{
    vec3 c = max(x, vec3(0.0));
    return c / (c + vec3(1.0));
}

vec3 LinearToSRGB(vec3 c)
{
    c = clamp(c, 0.0, 1.0);
    vec3 low = c * 12.92;
    vec3 high = 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055;
    return mix(low, high, step(vec3(0.0031308), c));
}

void main()
{
    vec3 hdr = max(texture(hdrTex, inTexCoord).rgb, vec3(0.0));

    if (globalUniforms.debugView != 0u)
    {
        bool needsGamma = (globalUniforms.debugView == 1u || globalUniforms.debugView == 8u);
        vec3 outRgb = needsGamma ? LinearToSRGB(hdr) : clamp(hdr, 0.0, 1.0);
        outColor = vec4(outRgb, 1.0);
        return;
    }

    vec3 mapped = hdr;
    if (globalUniforms.toneMapMode == 0u)
        mapped = ToneMapAgX(hdr);
    else if (globalUniforms.toneMapMode == 1u)
        mapped = ToneMapACES(hdr);
    else
        mapped = ToneMapReinhard(hdr);

    outColor = vec4(LinearToSRGB(mapped), 1.0);
}
