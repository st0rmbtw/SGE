static const float TAU = 6.283185307179586;
static const float AA_RADIUS = 0.8;

#if 0
// A heat map color for debugging (blue -> cyan -> green -> yellow -> red).
float4 heat_map_color(float value, float minValue, float maxValue, float2 position) {
    // Normalize value to 0-1 range
    let t = clamp((value - minValue) / (maxValue - minValue), 0.0, 1.0);

    // Heat map color calculation
    let r = t * t;
    let g = 4.0 * t * (1.0 - t);
    let b = (1.0 - t) * (1.0 - t);
    let heat_color = float3(r, g, b);

    // Create a checkerboard pattern (black and white)
    let sum = floor(position.x / 3) + floor(position.y / 3);
    let is_odd = fract(sum * 0.5); // 0.0 for even, 0.5 for odd
    let checker_value = is_odd * 2.0; // 0.0 for even, 1.0 for odd
    let checker_color = float3(checker_value);

    // Determine if value is in range (1.0 if in range, 0.0 if out of range)
    let in_range = step(minValue, value) * step(value, maxValue);

    // Mix checkerboard and heat map based on whether value is in range
    let final_color = lerp(checker_color, heat_color, in_range);

    return float4(final_color, 1.0);
}
#endif

float sdf_circle(float2 p, float radius) {
    return length(p) - radius;
}

float mod(float x, float y) {
    return x - y * floor(x/y);
}

float sdf_arc(in float2 p, in float a0, in float a1, in float r )
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

// corner_radii:
//    x: top left, y: top right, z: bottom left, w: bottom right
float pick_corner_radius(float2 center_to_point, float4 corner_radii) {
    if (center_to_point.x < 0.0) {
        if (center_to_point.y < 0.0) {
            return corner_radii.x;
        } else {
            return corner_radii.z;
        }
    } else {
        if (center_to_point.y < 0.0) {
            return corner_radii.y;
        } else {
            return corner_radii.w;
        }
    }
}

// Exact for r == 0 as well (gives a true Euclidean distance at corners).
float sdf_rounded_box(float2 p, float2 halfSize, float r)
{
    float2 q = abs(p) - halfSize + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

// corner_radii:
//    x: top left, y: top right, z: bottom left, w: bottom right
float4 sdf_rect(float2 p, float2 size, float4 color, float4 cornerRadii, float borderThickness, float4 borderColor)
{
    let dpdx = ddx(p);
    let dpdy = ddy(p);
    let pixelSize = max(sqrt(0.5 * (dot(dpdx, dpdx) + dot(dpdy, dpdy))), 1e-6);

    let halfSize = size * 0.5;
    let r = min(pick_corner_radius(p, cornerRadii), min(halfSize.x, halfSize.y));

    let outerSdf = sdf_rounded_box(p, halfSize, r);
    // let outerCov = saturate(0.5 - outerSdf / pixelSize);
    var outerCov = smoothstep(-AA_RADIUS, AA_RADIUS, -outerSdf / pixelSize);
    outerCov = pow(outerCov, 1.0 / 2.2);

    // Work in premultiplied alpha so interpolation is correct and there is no divide.
    let fill = float4(color.rgb * color.a, color.a);
    var result = fill;

    if (borderThickness > 0.0) {
        let bt = min(borderThickness, min(halfSize.x, halfSize.y));
        let innerSdf = sdf_rounded_box(p, halfSize - bt, max(r - bt, 0.0));
        // let fillCov  = saturate(0.5 - innerSdf / pixelSize);
        var fillCov  = smoothstep(-AA_RADIUS, AA_RADIUS, -innerSdf / pixelSize);
        fillCov = pow(fillCov, 1.0 / 2.2);

        let border = float4(borderColor.rgb * borderColor.a, borderColor.a);
        let borderOverFill = border + fill * (1.0 - border.a);
        result = lerp(borderOverFill, fill, fillCov);
    }

    result *= outerCov;

    // Back to straight alpha
    let a = result.a;
    return float4(a > 0.0 ? result.rgb / a : float3(0.0), a);
}

// Pixels per local unit along `dir` at `pos`. Valid for any projection.
float pixels_per_unit(float4x4 proj, float2 pos, float2 dir, float2 viewport_size)
{
    let c = mul(proj, float4(pos, 0.0, 1.0));
    let d = mul(proj, float4(dir, 0.0, 0.0)); // w = 0: linear part only, no translation

    let ndc0 = c.xy / c.w;
    let ndc1 = (c.xy + d.xy) / (c.w + d.w); // same as projecting pos + dir

    return length((ndc1 - ndc0) * 0.5 * viewport_size);
}