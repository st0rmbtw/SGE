// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const half3x3 RGB_TO_RRT = {
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const half3x3 ODT_TO_RGB = {
    1.60475, -0.53108, -0.07367,
    -0.10208, 1.10813, -0.00605,
    -0.00327, -0.07276, 1.07602
};

float3 RRTAndODTFit(float3 v) {
    let a = v * (v + 0.0245786) - 0.000090537;
    let b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

float3 ACESFitted(float3 color) {
    color = mul(RGB_TO_RRT, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = mul(ODT_TO_RGB, color);

    // Clamp to [0, 1]
    return saturate(color);
}

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : UV;
};

struct TonemapUniforms {
    float exposure;
};

[vk::binding(3)]
cbuffer TonemapConstantBuffer : register( b3 )
{
    TonemapUniforms uniforms;
};

[vk::binding(4)]
Texture2D MainTexture : register(t4);

[vk::binding(5)]
SamplerState MainSampler : register(s5);

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    float4 color = MainTexture.SampleLevel(MainSampler, inp.uv, 0);
    return float4(saturate(ACESFitted(color.rgb * uniforms.exposure)), color.a);
}