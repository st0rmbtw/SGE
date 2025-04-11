#version 450 core

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec2 v_uv;
layout(location = 1) flat in vec4 v_color;
layout(location = 2) flat in vec2 v_size;
layout(location = 3) flat in vec4 v_border_color;
layout(location = 4) flat in vec4 v_border_radius;
layout(location = 5) flat in float v_border_thickness;
layout(location = 6) flat in uint v_shape;

const float CIRCLE_AA = 0.001;

vec4 circle(in vec2 st, in vec4 color, in vec4 border_color, float border_thickness) {
    vec2 dist = st - vec2(0.5, 0.5);
    float d = dot(dist, dist);
    
    float border_cr = 0.5 - border_thickness * 0.5;
    vec2 border_weight = vec2(border_cr*border_cr+border_cr*CIRCLE_AA,border_cr*border_cr-border_cr*CIRCLE_AA);

    float cr = 0.5;
    vec2 weight = vec2(cr*cr+cr*CIRCLE_AA,cr*cr-cr*CIRCLE_AA);

    float t1 = 1.0 - clamp((d-border_weight.y)/(border_weight.x-border_weight.y),0.0,1.0);
    float t2 = 1.0 - clamp((d-weight.y)/(weight.x-weight.y),0.0,1.0);

    return vec4(mix(border_color.rgb, color.rgb, t1), t2);
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

// radius = [topLeft, topRight, bottomLeft, bottomRight]
float rounded_box_sdf(vec2 uv, vec2 size, vec4 radius) {
    vec2 center = size * (uv - 0.5);

    radius.xy = (center.x < 0.0) ? radius.xz : radius.yw;
    radius.x  = (center.y < 0.0) ? radius.x  : radius.y;

    vec2 dist = abs(center) - size * 0.5 + radius.x;
    return min(max(dist.x,dist.y),0.0) + length(max(dist, 0.0)) - radius.x;
}

const uint SHAPE_CIRCLE = 1u;
const uint SHAPE_ARC = 2u;

void main() {
    frag_color = v_color;
    
    if (v_shape == SHAPE_CIRCLE) {
        frag_color = circle(v_uv, v_color, v_border_color, v_border_thickness);
    } else if (v_shape == SHAPE_ARC) {
        float start_angle = v_border_radius.x;
        float end_angle = v_border_radius.y;

        float thickness = 0.5 - v_border_thickness / v_size.y;

        vec2 p = ((vec2(v_uv.x, 1.0 - v_uv.y) * 2.0 - 1.0) * v_size) / v_size.y;

        float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
        float aa = length(vec2(dFdx(d), dFdy(d)));

        float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);

        frag_color = vec4(v_color.rgb, min(v_color.a, alpha));
    } else {
        float radius = max(v_border_radius.x, max(v_border_radius.y, max(v_border_radius.z, v_border_radius.w)));

        if (radius > 0.0) {
            float d = rounded_box_sdf(v_uv, v_size, v_border_radius);
            float aa = length(vec2(dFdx(d), dFdy(d)));

            float smoothed_alpha = 1.0 - smoothstep(0.0-aa, 0.0, d);
            float border_alpha = 1.0 - smoothstep(v_border_thickness - aa, v_border_thickness, abs(d));

            vec4 quad_color = vec4(v_color.rgb, min(v_color.a, smoothed_alpha));
            vec4 quad_color_with_border = mix(quad_color, v_border_color, min(v_border_color.a, min(border_alpha, smoothed_alpha)));

            vec4 result = vec4(quad_color_with_border.rgb, mix(0.0, quad_color_with_border.a, smoothed_alpha));

            if (result.a < 0.01) discard;
            
            frag_color = result;
        }
    }
}