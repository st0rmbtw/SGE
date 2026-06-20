struct VSInput
{
    float3 pos : Position;
};

struct VSOutput
{
    float4 position : SV_Position;
};

struct Uniforms
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 color;
};

[vk::binding(2)]
cbuffer UniformConstantBuffer : register(b2)
{
    Uniforms uniforms;
};

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    VSOutput outp;
    outp.position = mul(mul(uniforms.projectionMatrix, uniforms.viewMatrix), float4(inp.pos, 1.0));
    return outp;
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return float4(uniforms.color, 1.0);
}