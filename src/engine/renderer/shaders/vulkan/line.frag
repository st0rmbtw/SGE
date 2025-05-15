#version 450 core

layout(location = 0) out vec4 frag_color;

layout(location = 0) flat in vec4 v_color;
layout(location = 1) flat in vec4 v_border_radius;
layout(location = 2) in vec2 v_point;
layout(location = 3) in vec2 v_uv;
layout(location = 4) flat in vec2 v_size;

// The returned value is the shortest distance from the given point to the boundary of the rounded 
// box.
// 
// Negative values indicate that the point is inside the rounded box, positive values that the point 
// is outside, and zero is exactly on the boundary.
//
// Arguments: 
//  - `point`        -> The function will return the distance from this point to the closest point on 
//                    the boundary.
//  - `size`         -> The maximum width and height of the box.
//  - `corner_radii` -> The radius of each rounded corner. Ordered counter clockwise starting 
//                    top left:
//                      x: top left, y: top right, z: bottom left, w: bottom right.
float sd_rounded_box(vec2 point, vec2 size, vec4 corner_radii) {
    // If 0.0 < y then select bottom left (z) and bottom right corner radius (w).
    // Else select top left (x) and top right corner radius (y).
    vec2 rs = 0.0 < point.y ? corner_radii.zw : corner_radii.xy;
    // w and z are swapped above so that both pairs are in left to right order, otherwise this second 
    // select statement would return the incorrect value for the bottom pair.
    float radius = 0.0 < point.x ? rs.y : rs.x;
    // Vector from the corner closest to the point, to the point.
    vec2 corner_to_point = abs(point) - 0.5 * size;
    // Vector from the center of the radius circle to the point.
    vec2 q = corner_to_point + radius;
    // Length from center of the radius circle to the point, zeros a component if the point is not 
    // within the quadrant of the radius circle that is part of the curved corner.
    float l = length(max(q, vec2(0.0)));
    float m = min(max(q.x, q.y), 0.0);
    return l + m - radius;
}

float sd_inset_rounded_box(vec2 point, vec2 size, vec4 radius, vec4 inset) {
    vec2 inner_size = size - inset.xy - inset.zw;
    vec2 inner_center = inset.xy + 0.5 * inner_size - 0.5 * size;
    vec2 inner_point = point - inner_center;

    vec4 r = radius;

    // Top left corner.
    r.x = r.x - max(inset.x, inset.y);

    // Top right corner.
    r.y = r.y - max(inset.z, inset.y);

    // Bottom right corner.
    r.z = r.z - max(inset.z, inset.w); 

    // Bottom left corner.
    r.w = r.w - max(inset.x, inset.w);

    vec2 half_size = inner_size * 0.5;
    float min_size = min(half_size.x, half_size.y);

    r = min(max(r, vec4(0.0)), vec4(min_size));

    return sd_rounded_box(inner_point, inner_size, r);
}

// get alpha for antialiasing for sdf
float antialias(float d) {
    // return clamp(0.5 - d, 0.0, 1.0);
    // float aa = length(vec2(dFdx(d), dFdy(d)));
    // float aa = pow(fwidth(d), 1.0 / 2.2);
    float aa = pow(fwidth(d), 1.0 / 2.2);
    return 1.0 - smoothstep(-aa, 0.0, d);
}

void main() {
    vec4 result = v_color;

    float radius = max(v_border_radius.x, max(v_border_radius.y, max(v_border_radius.z, v_border_radius.w)));

    if (radius > 0.0) {
        float external_distance = sd_rounded_box(v_point, v_size, v_border_radius);
        float smoothed_alpha = antialias(external_distance);
        result = vec4(v_color.rgb, min(v_color.a, smoothed_alpha));
    }

    if (result.a <= 0.05) discard;

    frag_color = result;
}