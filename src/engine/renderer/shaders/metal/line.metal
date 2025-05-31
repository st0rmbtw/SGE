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
    float2 position        [[attribute(0)]];
    float2 i_start         [[attribute(1)]];
    float2 i_end           [[attribute(2)]];
    float4 i_color         [[attribute(3)]];
    float4 i_border_radius [[attribute(4)]];
    float  i_thickness     [[attribute(5)]];
    uint   i_flags         [[attribute(6)]];
};

struct VertexOut
{
    float4 position [[position]];
    float4 color [[flat]];
    float4 border_radius [[flat]];
    float2 size [[flat]];
    float2 point;
    float2 uv;
};

constant constexpr uint FLAG_UI = 1 << 0;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]],
    uint vid [[vertex_id]]
) {
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;
    const float4x4 mvp = is_ui ? constants.screen_projection : constants.view_projection;

    const float2 d = inp.i_end - inp.i_start;
    const float len = length(d);
    const float2 perp = (float2(d.y, -d.x) / len) * inp.i_thickness * 0.5;

    float2 vertices[4] = {
        float2(inp.i_start - perp),
        float2(inp.i_start + perp),
        float2(inp.i_end - perp),
        float2(inp.i_end + perp)
    };

    float2 size = float2(len, inp.i_thickness);

	VertexOut outp;
    outp.color = inp.i_color;
    outp.border_radius = inp.i_border_radius;
    outp.size = size;
    outp.point = (inp.position - 0.5) * size;
    outp.uv = inp.position;
    outp.position = mvp * float4(vertices[vid], 0.0, 1.0);
    outp.position.z = 1.0;

    return outp;
}

// The returned value is the shortest distance from the given point to the boundary of the rounded 
// box.
// 
// Negative values indicate that the point is inside the rounded box, positive values that the point 
// is outside, and zero is exactly on the boundary.
//
// Arguments: 
//  - `p`        -> The function will return the distance from this point to the closest point on 
//                    the boundary.
//  - `size`         -> The maximum width and height of the box.
//  - `corner_radii` -> The radius of each rounded corner. Ordered counter clockwise starting 
//                    top left:
//                      x: top left, y: top right, z: bottom left, w: bottom right.
float sd_rounded_box(float2 p, float2 size, float4 corner_radii) {
    // If 0.0 < y then select bottom left (z) and bottom right corner radius (w).
    // Else select top left (x) and top right corner radius (y).
    float2 rs = 0.0 < p.y ? corner_radii.zw : corner_radii.xy;
    // w and z are swapped above so that both pairs are in left to right order, otherwise this second 
    // select statement would return the incorrect value for the bottom pair.
    float radius = 0.0 < p.x ? rs.y : rs.x;
    // Vector from the corner closest to the point, to the point.
    float2 corner_to_point = abs(p) - 0.5 * size;
    // Vector from the center of the radius circle to the point.
    float2 q = corner_to_point + radius;
    // Length from center of the radius circle to the point, zeros a component if the point is not 
    // within the quadrant of the radius circle that is part of the curved corner.
    float l = length(max(q, float2(0.0, 0.0)));
    float m = min(max(q.x, q.y), 0.0);
    return l + m - radius;
}

// get alpha for antialiasing for sdf
float antialias(float d) {
    float aa = pow(fwidth(d), 1.0 / 2.2);
    return 1.0 - smoothstep(-aa, 0.0, d);
}

fragment float4 PS(
    VertexOut inp [[stage_in]]
) {
    float4 result = inp.color;

    float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));
    if (radius > 0.0) {
        float external_distance = sd_rounded_box(inp.point, inp.size, inp.border_radius);
        float smoothed_alpha = antialias(external_distance);
        result = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
    }

    if (result.a < 0.05) discard_fragment();

    return result;
}