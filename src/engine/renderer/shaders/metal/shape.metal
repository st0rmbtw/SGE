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
    float2 i_size [[attribute(2)]];
    float2 i_offset [[attribute(3)]];
    float4 i_color [[attribute(4)]];
    float4 i_border_color [[attribute(5)]];
    float4 i_border_radius[[attribute(6)]];
    float i_border_thickness [[attribute(7)]];
    uint i_shape [[attribute(8)]];
    uint i_flags [[attribute(9)]];
};

struct VertexOut
{
    float4 position [[position]];

    float2 uv;
    float2 size [[flat]];
    float4 color [[flat]];
    float4 border_color [[flat]];
    float4 border_radius [[flat]];
    float border_thickness [[flat]];
    uint shape [[flat]];
};

constant const uint FLAG_UI = 1 << 0;

constant const uint SHAPE_CIRCLE = 1;
constant const uint SHAPE_ARC = 2;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    // const float qxx = inp.i_rotation.x * inp.i_rotation.x;
    // const float qyy = inp.i_rotation.y * inp.i_rotation.y;
    // const float qzz = inp.i_rotation.z * inp.i_rotation.z;
    // const float qxz = inp.i_rotation.x * inp.i_rotation.z;
    // const float qxy = inp.i_rotation.x * inp.i_rotation.y;
    // const float qyz = inp.i_rotation.y * inp.i_rotation.z;
    // const float qwx = inp.i_rotation.w * inp.i_rotation.x;
    // const float qwy = inp.i_rotation.w * inp.i_rotation.y;
    // const float qwz = inp.i_rotation.w * inp.i_rotation.z;

    // const float4x4 rotation_matrix = float4x4(
    //     float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
    //     float4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
    //     float4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
    //     float4(0.0, 0.0, 0.0, 1.0)
    // );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(inp.i_position.x, inp.i_position.y, 0.0, 1.0)
    );

    // transform = transform * rotation_matrix;

    const float2 offset = -inp.i_offset * inp.i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * inp.i_size[0];
    transform[1] = transform[1] * inp.i_size[1];

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = (is_ui ? constants.screen_projection : constants.view_projection) * transform;
    const float2 position = inp.position;

    VertexOut outp;
    outp.position = mvp * float4(position, 0.0, 1.0);
    outp.position.z = inp.i_position.z;
    outp.uv = position;
    outp.size = inp.i_size;
    outp.color = inp.i_color;
    outp.border_color = inp.i_border_color;
    outp.border_thickness = inp.i_border_thickness;
    outp.border_radius = inp.i_border_radius;
    outp.shape = inp.i_shape;

    return outp;
}

constant constexpr float CIRCLE_AA = 0.001;

float4 circle(float2 st, float4 color, float4 border_color, float border_thickness) {
    float2 dist = st - float2(0.5, 0.5);
    float d = dot(dist, dist);
    
    float border_cr = 0.5 - border_thickness * 0.5;
    float2 border_weight = float2(border_cr*border_cr+border_cr*CIRCLE_AA,border_cr*border_cr-border_cr*CIRCLE_AA);

    float cr = 0.5;
    float2 weight = float2(cr*cr+cr*CIRCLE_AA,cr*cr-cr*CIRCLE_AA);

    float t1 = 1.0 - clamp((d-border_weight.y)/(border_weight.x-border_weight.y),0.0,1.0);
    float t2 = 1.0 - clamp((d-weight.y)/(weight.x-weight.y),0.0,1.0);

    return float4(mix(border_color.rgb, color.rgb, t1), t2);
}

constant constexpr float PI = 3.1415926535;
constant constexpr float TAU = 6.283185307179586;

float mod(float x, float y) {
    return x - y * floor(x/y);
}

float arc_sdf(float2 p, float a0, float a1, float r )
{
    float a = mod(atan2(p.y, p.x), TAU);

    float ap = a - a0;
    if (ap < 0.)
        ap += TAU;
    float a1p = a1 - a0;
    if (a1p < 0.)
        a1p += TAU;

    // is a outside [a0, a1]?
    // https://math.stackexchange.com/questions/1044905/simple-angle-between-two-angles-of-circle
    if (ap >= a1p) {
        // snap to the closest of the two endpoints
        float2 q0 = float2(r * cos(a0), r * sin(a0));
        float2 q1 = float2(r * cos(a1), r * sin(a1));
        return min(length(p - q0), length(p - q1));
    }

    return abs(length(p) - r);
}

// radius = [topLeft, topRight, bottomLeft, bottomRight]
float rounded_box_sdf(float2 uv, float2 size, float4 radius) {
    float2 center = size * (uv - 0.5);

    radius.xy = (center.x < 0.0) ? radius.xz : radius.yw;
    radius.x  = (center.y < 0.0) ? radius.x  : radius.y;

    float2 dist = abs(center) - size * 0.5 + radius.x;
    return min(max(dist.x,dist.y),0.0) + length(max(dist, 0.0)) - radius.x;
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    if (inp.shape == SHAPE_CIRCLE) {
        return circle(inp.uv, inp.color, inp.border_color, inp.border_thickness);
    } else if (inp.shape == SHAPE_ARC) {
        float start_angle = inp.border_radius.x;
        float end_angle = inp.border_radius.y;

        float thickness = 0.5 - inp.border_thickness / inp.size.y;

        float2 p = ((float2(inp.uv.x, 1.0 - inp.uv.y) * 2.0 - 1.0) * inp.size) / inp.size.y;

        float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
        float aa = length(float2(dfdx(d), dfdy(d)));

        float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);

        return float4(inp.color.rgb, min(inp.color.a, alpha));
    }

    float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));

    if (radius > 0.0) {
        float d = rounded_box_sdf(inp.uv, inp.size, inp.border_radius);
        float aa = length(float2(dfdx(d), dfdy(d)));

        float smoothed_alpha = 1.0 - smoothstep(0.0-aa, 0.0, d);
        float border_alpha = 1.0 - smoothstep(inp.border_thickness - aa, inp.border_thickness, abs(d));

        float4 quad_color = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
        float4 quad_color_with_border = mix(quad_color, inp.border_color, min(inp.border_color.a, min(border_alpha, smoothed_alpha)));

        float4 result = float4(quad_color_with_border.rgb, mix(0.0, quad_color_with_border.a, smoothed_alpha));

        if (result.a < 0.01) discard_fragment();
        
        return result;
    }

    return inp.color;
}