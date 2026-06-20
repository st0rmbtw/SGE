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

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : UV;
};

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return MainTexture.SampleLevel(MainSampler, inp.uv, 0) * uniforms.intensity;
}