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
    float4 color      [[attribute(1)]];
    uint   flags      [[attribute(2)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 color [[flat]];
};

constant constexpr uint FLAG_UI = 1 << 0;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const bool is_ui = (inp.flags & FLAG_UI) == FLAG_UI;
    const float4x4 mvp = is_ui ? constants.screen_projection : constants.view_projection;

	VertexOut outp;
    outp.color = inp.color;
    outp.position = mvp * float4(inp.position, 0.0, 1.0);
    outp.position.z = 1.0;

    return outp;
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    if (inp.color.a < 0.05) discard_fragment();

    return inp.color;
}