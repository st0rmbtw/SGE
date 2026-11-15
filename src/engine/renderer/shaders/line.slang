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
    let proj  = is_ui ? uniforms.screen_projection : uniforms.view_projection;

    let d   = inp.i_end - inp.i_start;
    let len = length(d);
    let dir = len > 1e-6 ? d / len : float2(1.0, 0.0);
    let nrm = float2(dir.y, -dir.x);

    let size   = float2(len, inp.i_thickness);
    let center = (inp.i_start + inp.i_end) * 0.5;

    let local0 = (inp.position - 0.5) * size;
    let pos0   = center + dir * local0.x + nrm * local0.y;

    let pad = float2(
        ceil(AA_RADIUS*2) / max(pixels_per_unit(proj, pos0, dir, uniforms.window_size), 1e-6),
        ceil(AA_RADIUS*2) / max(pixels_per_unit(proj, pos0, nrm, uniforms.window_size), 1e-6),
    );

    let local = (inp.position - 0.5) * (size + 2.0 * pad);
    let world = center + dir * local.x + nrm * local.y;

    VSOutput outp;
    outp.position    = mul(proj, float4(world, 0.0, 1.0));
    outp.position.z  = 1.0;
    outp.color       = inp.i_color;
    outp.cornerRadii = inp.i_corner_radii;
    outp.size        = size;
    outp.p           = local; // extends slightly beyond ±size/2
    return outp;
}

static const float EPSILON = 0.0001f;

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    return sdf_rect(inp.p, inp.size, inp.color, inp.cornerRadii, 0.0, float4(0.0));
}
