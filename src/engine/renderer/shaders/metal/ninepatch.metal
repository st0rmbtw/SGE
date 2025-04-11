#include <metal_stdlib>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 nozoom_view_projection;
    float4x4 nozoom_projection;
    float4x4 transform_matrix;
    float4x4 inv_view_proj;
    float2 camera_position;
    float2 window_size;
};

struct VertexIn
{
    float2 position [[attribute(0)]];

    float3 i_position [[attribute(1)]];
    float4 i_rotation [[attribute(2)]];
    float2 i_size [[attribute(3)]];
    float2 i_offset [[attribute(4)]];
    float2 i_source_size[[attribute(5)]];
    float2 i_output_size[[attribute(6)]];
    uint4  i_margin [[attribute(7)]];
    float4 i_uv_offset_scale [[attribute(8)]];
    float4 i_color [[attribute(9)]];
    uint i_flags [[attribute(10)]];
};

struct VertexOut
{
    float4 position [[position]];

    float4 color;
    uint4  margin [[flat]];
    float2 source_size [[flat]];
    float2 output_size [[flat]];
    float2 uv;
};

constant constexpr uint FLAG_UI = 1 << 0;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const float qxx = inp.i_rotation.x * inp.i_rotation.x;
    const float qyy = inp.i_rotation.y * inp.i_rotation.y;
    const float qzz = inp.i_rotation.z * inp.i_rotation.z;
    const float qxz = inp.i_rotation.x * inp.i_rotation.z;
    const float qxy = inp.i_rotation.x * inp.i_rotation.y;
    const float qyz = inp.i_rotation.y * inp.i_rotation.z;
    const float qwx = inp.i_rotation.w * inp.i_rotation.x;
    const float qwy = inp.i_rotation.w * inp.i_rotation.y;
    const float qwz = inp.i_rotation.w * inp.i_rotation.z;

    const float4x4 rotation_matrix = float4x4(
        float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
        float4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
        float4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(inp.i_position.x, inp.i_position.y, 0.0, 1.0)
    );

    transform = transform * rotation_matrix;

    const float2 offset = -inp.i_offset * inp.i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * inp.i_size[0];
    transform[1] = transform[1] * inp.i_size[1];

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = (is_ui ? constants.screen_projection : constants.view_projection) * transform;
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VertexOut outp;
    outp.position = mvp * float4(inp.position, 0.0, 1.0);
    outp.position.z = 1.0;
    outp.uv = inp.position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.margin = inp.i_margin;
    outp.source_size = inp.i_source_size;
    outp.output_size = inp.i_output_size;

    return outp;
}

constant constexpr float GLYPH_CENTER = 0.5;

float map(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
} 

float process_axis(float coord, float2 source_margin, float2 out_margin) {
    if (coord < out_margin.x) {
        return map(coord, 0.0f, out_margin.x, 0.0f, source_margin.x);
    }
    if (coord < 1.0 - out_margin.y) {
        return map(coord, out_margin.x, 1.0 - out_margin.y, source_margin.x, 1.0 - source_margin.y);
    }
    return map(coord, 1.0 - out_margin.y, 1.0, 1.0 - source_margin.y, 1.0);
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    const float2 horizontal_margin = inp.margin.xy;
    const float2 vertical_margin = inp.margin.zw;

    const float2 new_uv = float2(
        process_axis(inp.uv.x,
            horizontal_margin / inp.source_size.xx,
            horizontal_margin / inp.output_size.xx
        ),
        process_axis(inp.uv.y,
            vertical_margin / inp.source_size.yy,
            vertical_margin / inp.output_size.yy
        )
    );

    const float4 color = texture.sample(texture_sampler, new_uv) * inp.color;

    if (color.a < 0.05) discard_fragment();

    return color;
}