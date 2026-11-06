#include "common.slang"
#include "sdf.slang"

[vk::binding(2)]
cbuffer GlobalUniformBuffer : register( b2 )
{
    GlobalUniforms uniforms;
};

struct VSInput
{
    float2 position : Position;
    float2 i_start : I_Start;
    float2 i_end : I_End;
    float4 i_color : I_Color;
    float4 i_corner_radii : I_CornerRadii;
    float i_thickness : I_Thickness;
    uint i_flags : I_Flags;

    uint vid : SV_VertexID;
};

struct VSOutput
{
    float4 position : SV_Position;
    nointerpolation float4 color : Color;
    nointerpolation float4 cornerRadii : CornerRadii;
    nointerpolation float2 size : Size;
    float2 p : Point;
};

static const uint FLAG_UI = 1 << 0;

[shader("vertex")]
VSOutput VS(VSInput inp)
{
    let is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    let mvp = is_ui ? uniforms.screen_projection : uniforms.view_projection;

    let d = inp.i_end - inp.i_start;
    let len = length(d);
    let perp = (float2(d.y, -d.x) / len) * inp.i_thickness * 0.5;

    let vertices: float2[] = {
        float2(inp.i_start - perp),
        float2(inp.i_start + perp),
        float2(inp.i_end - perp),
        float2(inp.i_end + perp)
    };

    float2 size = float2(len, inp.i_thickness);

    VSOutput outp;
    outp.color = inp.i_color;
    outp.cornerRadii = inp.i_corner_radii;
    outp.size = size;
    outp.p = (inp.position - 0.5) * size;
    outp.position = mul(mvp, float4(vertices[inp.vid], 0.0, 1.0));
    outp.position.z = 1.0;

	return outp;
}

static const float EPSILON = 0.0001f;

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return sdf_rect(inp.p, inp.size, inp.color, inp.cornerRadii, 0.0, float4(0.0));
}