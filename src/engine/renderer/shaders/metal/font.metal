#include <metal_stdlib>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 nonscale_view_projection;
    float4x4 nonscale_projection;
    float4x4 transform_matrix;
    float4x4 inv_view_proj;
    float2 camera_position;
    float2 window_size;
};

struct VertexIn
{
    float2 position   [[attribute(0)]];

    float3 i_color    [[attribute(1)]];
    float2 i_position [[attribute(2)]];
    float2 i_size     [[attribute(3)]];
    float2 i_tex_size [[attribute(4)]];
    float2 i_uv       [[attribute(5)]];
    uint   i_flags    [[attribute(6)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 color [[flat]];
    float2 tex_size [[flat]];
    float2 uv;
};

constant constexpr uint FLAG_UI = 1 << 0;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? constants.screen_projection : constants.view_projection;

    const float2 position = inp.i_position + inp.position * inp.i_size;
    const float2 uv = inp.i_uv + inp.position * inp.i_tex_size;

	VertexOut outp;
    outp.color = inp.i_color;
    outp.uv = uv;
    outp.tex_size = inp.i_tex_size;
    outp.position = mvp * float4(position, 0.0, 1.0);
    outp.position.z = 1.0;

    return outp;
}

constant const float GLYPH_CENTER = 0.5;

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    const float outline = 0.2;

    const float dist = texture.sample(texture_sampler, inp.uv).r;
    const float width = fwidth(dist);
    const float alpha = smoothstep(GLYPH_CENTER - outline - width, GLYPH_CENTER - outline + width, abs(dist));
    // float4 color = float4(inp.color, alpha);

    const float mu = smoothstep(0.5-width, 0.5+width, abs(dist));
    const float3 rgb = lerp(float3(0.0, 0.0, 0.0), inp.color, mu);
    const float4 color = float4(rgb, alpha);

    if (color.a < 0.05) discard_fragment();

    return color;
}