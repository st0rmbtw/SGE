cbuffer GlobalUniformBuffer : register( b2 )
{
    float4x4 u_screen_projection;
    float4x4 u_view_projection;
    float4x4 u_nonscale_view_projection;
    float4x4 u_nonscale_projection;
    float4x4 u_transform_matrix;
    float4x4 u_inv_view_proj;
    float2 u_camera_position;
    float2 u_window_size;
};

struct VSInput
{
    uint vid : SV_VertexID;

    float2 position : Position;
    float2 i_start : I_Start;
    float2 i_end : I_End;
    float4 i_color : I_Color;
    float4 i_border_radius : I_Border_Radius;
    float i_thickness : I_Thickness;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    nointerpolation float4 color : Color;
    nointerpolation float4 border_radius : BorderRadius;
    nointerpolation float2 size : Size;
    float2 p : Point;
    float2 uv : UV;
};

static const uint FLAG_UI = 1 << 0;

VSOutput VS(VSInput inp)
{
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? u_screen_projection : u_view_projection;

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

    VSOutput outp;
    outp.color = inp.i_color;
    outp.border_radius = inp.i_border_radius;
    outp.size = size;
    outp.p = (inp.position - 0.5) * size;
    outp.uv = inp.position;
    outp.position = mvp * float4(vertices[inp.vid], 0.0, 1.0);
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

float4 PS(VSOutput inp) : SV_Target
{
    float4 result = inp.color;

    float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));
    if (radius > 0.0) {
        float external_distance = sd_rounded_box(inp.p, inp.size, inp.border_radius);
        float smoothed_alpha = antialias(external_distance);
        result = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
    }

    clip(result.a - 0.05);

    return result;
}