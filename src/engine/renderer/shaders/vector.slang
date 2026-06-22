#include "common.slang"

[vk::binding(2)]
cbuffer GlobalUniformBuffer : register( b2 )
{
    GlobalUniforms uniforms;
};

[vk::binding(3)]
cbuffer PathDataBuffer : register(b3)
{
    float4x4 path_transform;
    float4 path_fill_color;
};

struct VSInput
{
    float2 position : Position;
};

struct VSOutput
{
    float4 position : SV_Position;
};

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    float4x4 mvp = mul(uniforms.view_projection, path_transform);

    VSOutput outp;
    outp.position = mul(mvp, float4(inp.position.xy, 0.f, 1.f));
    return outp;
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return path_fill_color;
}