[vk::binding(2)]
cbuffer UniformBuffer : register( b2 )
{
    float4x4 ProjectionMatrix;
};

struct VSInput
{
    float2 position : Position;
    float2 uv  : UV;
    float4 color : Color;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : Color;
    float2 uv : UV;
};

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    VSOutput outp;
    outp.position = mul(ProjectionMatrix, float4(inp.position.xy, 0.f, 1.f));
    outp.color = inp.color;
    outp.uv  = inp.uv;
    return outp;
}

[vk::binding(3)]
Texture2D Texture : register(t3);

[vk::binding(4)]
SamplerState Sampler : register(s4);

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return inp.color * Texture.Sample(Sampler, inp.uv);
}