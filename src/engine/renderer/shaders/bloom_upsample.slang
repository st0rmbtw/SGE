struct VSOutput {
    float4 position : SV_Position;
    float2 uv : UV;
};

struct BloomUniforms {
    float threshold;
    float knee;
    float filterRadius;
    float intensity;
};

[vk::binding(3)]
cbuffer BloomConstantBuffer : register( b3 )
{
    BloomUniforms uniforms;
};

[vk::binding(4)]
Texture2D MainTexture : register(t4);

[vk::binding(5)]
SamplerState MainSampler : register(s5);

float4 UpsampleTent(float2 uv, float2 texelSize, float sampleScale)
{
    float4 d = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

    float4 s;
    s =  MainTexture.Sample(MainSampler, uv - d.xy);
    s += MainTexture.Sample(MainSampler, uv - d.wy) * 2.0;
    s += MainTexture.Sample(MainSampler, uv - d.zy);

    s += MainTexture.Sample(MainSampler, uv + d.zw) * 2.0;
    s += MainTexture.Sample(MainSampler, uv       ) * 4.0;
    s += MainTexture.Sample(MainSampler, uv + d.xw) * 2.0;

    s += MainTexture.Sample(MainSampler, uv + d.zy);
    s += MainTexture.Sample(MainSampler, uv + d.wy) * 2.0;
    s += MainTexture.Sample(MainSampler, uv + d.xy);

    return s * (1.0 / 16.0);
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    float width;
    float height;
    MainTexture.GetDimensions(width, height);

    float2 texelSize = 1.0 / float2(width, height);

    return UpsampleTent(inp.uv, texelSize, uniforms.filterRadius);
}