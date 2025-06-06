#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;
in vec2 v_point;
flat in vec4 v_color;
flat in vec2 v_size;
flat in vec4 v_border_color;
flat in vec4 v_border_radius;
flat in float v_border_thickness;
flat in uint v_shape;

const float CIRCLE_AA = 0.001;

float sdf_circle(vec2 p, float radius) {
    return length(p) - radius;
}

const float PI = 3.1415926535;
const float TAU = 6.283185307179586;

float modb(float x, float y) {
    return x - y * floor(x/y);
}

float arc_sdf(in vec2 p, in float a0, in float a1, in float r )
{
    float a = modb(atan(p.y, p.x), TAU);

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
        vec2 q0 = vec2(r * cos(a0), r * sin(a0));
        vec2 q1 = vec2(r * cos(a1), r * sin(a1));
        return min(length(p - q0), length(p - q1));
    }

    return abs(length(p) - r);
}

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

const uint SHAPE_CIRCLE = 1u;
const uint SHAPE_ARC = 2u;

// get alpha for antialiasing for sdf
float antialias(float d) {
    // return clamp(pow(0.5 - d, 1.0 / 2.2), 0.0, 1.0);

    float afwidth = pow(fwidth(d), 1.0 / 2.2);
    return 1.0 - smoothstep(0.0, afwidth, d);
}

float antialias_circle(float d) {
    float afwidth = fwidth(d) * 1.3;
    return 1.0 - smoothstep(0.0, afwidth, d);
}

void main() {
    vec4 result = v_color;

    if (v_shape == SHAPE_CIRCLE) {
        float radius = 0.5 - 1.0 / v_size.x;
        float external_distance = sdf_circle(v_uv - 0.5, radius);
        float alpha = antialias_circle(external_distance);

        if (v_border_thickness > 0.0) {
            float internal_distance = sdf_circle(v_uv - 0.5, radius - v_border_thickness / v_size.x);

            float border_distance = max(external_distance, -internal_distance);

            float border_alpha = antialias_circle(border_distance);

            vec4 color = vec4(v_color.rgb, min(v_color.a, alpha));
            vec4 color_with_border = mix(color, v_border_color, min(v_border_color.a, min(border_alpha, alpha)));

            result = internal_distance > 0.0 && border_alpha < 1.0
                ? mix(vec4(0.0), v_border_color, border_alpha)
                : color_with_border;
        } else {
            result = vec4(v_color.rgb, min(v_color.a, alpha));
        }
    } else if (v_shape == SHAPE_ARC) {
        float start_angle = v_border_radius.x;
        float end_angle = v_border_radius.y;

        float thickness = 0.5 - v_border_thickness / v_size.y;

        vec2 p = ((vec2(v_uv.x, 1.0 - v_uv.y) * 2.0 - 1.0) * v_size) / v_size.y;

        float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
        float aa = length(vec2(dFdx(d), dFdy(d)));

        float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);

        result = vec4(v_color.rgb, min(v_color.a, alpha));
    } else {
        float radius = max(v_border_radius.x, max(v_border_radius.y, max(v_border_radius.z, v_border_radius.w)));

        if (radius > 0.0 || v_border_thickness > 0.0) {
            // Signed distance from the exterior boundary.
            float external_distance = sd_rounded_box(v_point, v_size, v_border_radius);

            // Signed distance from the border's internal edge (the signed distance is negative if the point
            // is inside the rect but not on the border).
            // If the border size is set to zero, this is the same as the external distance.
            float internal_distance = sd_inset_rounded_box(v_point, v_size, v_border_radius, vec4(v_border_thickness));

            // Signed distance from the border (the intersection of the rect with its border).
            // Points inside the border have negative signed distance. Any point outside the border, whether
            // outside the outside edge, or inside the inner edge have positive signed distance.
            float border_distance = max(external_distance, -internal_distance);

            float border_alpha = antialias(border_distance);

            float smoothed_alpha = antialias(external_distance);

            vec4 quad_color = vec4(v_color.rgb, min(v_color.a, smoothed_alpha));
            vec4 quad_color_with_border = mix(quad_color, v_border_color, min(v_border_color.a, min(border_alpha, smoothed_alpha)));

            result = internal_distance > 0.0 && border_alpha < 1.0
                ? mix(vec4(0.0), v_border_color, border_alpha)
                : quad_color_with_border;
        }
    }

    if (result.a < 0.01) discard;

    frag_color = result;
}