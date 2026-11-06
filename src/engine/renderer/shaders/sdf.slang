static const float TAU = 6.283185307179586;

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

float4 over(float4 below, float4 above) {
    let alpha = above.a + below.a * (1.0 - above.a);
    let color = (above.rgb * above.a + below.rgb * below.a * (1.0 - above.a)) / alpha;
    return float4(color, alpha);
}

// corner_radii:
//    x: top left, y: top right, z: bottom left, w: bottom right
float4 sdf_rect(float2 point, float2 size, float4 color, float4 cornerRadii, float borderThickness, float4 borderColor) {
    let isUnrounded = all(cornerRadii == 0.0);
    if (borderThickness == 0.0 && isUnrounded) {
        return color;
    }

    // Signed distance field threshold for inclusion of pixels. 0.5 is the
    // minimum distance between the center of the pixel and the edge.
    let antialiasThreshold = 0.5;

    let reducedBorder = select(borderThickness == 0.0, -antialiasThreshold, borderThickness);

    // Radius of the nearest corner
    let cornerRadius = pick_corner_radius(point, cornerRadii);

    let cornerToPoint = abs(point) - size * 0.5;

    let cornerCenterToPoint = cornerToPoint + cornerRadius;

    let isNearRoundedCorner =
        cornerCenterToPoint.x >= 0 &&
        cornerCenterToPoint.y >= 0;

    let straightBorderInnerCornerToPoint = cornerToPoint + reducedBorder;

    let isBeyondInnerStraightBorder =
        straightBorderInnerCornerToPoint.x > 0 ||
        straightBorderInnerCornerToPoint.y > 0;

    // Whether the point is far enough inside the quad, such that the pixels are
    // not affected by the straight border.
    let isWithinInnerStraightBorder =
        straightBorderInnerCornerToPoint.x < (-antialiasThreshold) &&
        straightBorderInnerCornerToPoint.y < (-antialiasThreshold);

    // Fast path for points that must be part of the background.
    if (isWithinInnerStraightBorder && !isNearRoundedCorner) {
        return color;
    }

    let pixelSize = max(length(float2(ddx(point.x), ddy(point.y))), 1e-5);

    // Signed distance of the point to the outside edge of the quad's border. It
    // is positive outside this edge, and negative inside.
    let outer_sdf = sdf_rect_impl(cornerCenterToPoint, cornerRadius);

    var inner_sdf = 0.0;
    if (cornerCenterToPoint.x <= 0 || cornerCenterToPoint.y <= 0) {
        // Fast paths for straight borders.
        inner_sdf = -max(straightBorderInnerCornerToPoint.x,
                            straightBorderInnerCornerToPoint.y);
    } else if (isBeyondInnerStraightBorder) {
        // Fast path for points that must be outside the inner edge.
        inner_sdf = -borderThickness;
    } else { // if (reducedBorder == reducedBorder)
        // Fast path for circular inner edge.
        inner_sdf = -(outer_sdf + reducedBorder);
    }
    // else {
    //     let ellipse_radii = max(float2(0.0, 0.0), cornerRadius - reducedBorder);
    //     inner_sdf = sdf_quarter_ellipse(cornerCenterToPoint, ellipse_radii);
    // }

    // Negative when inside the border
    let border_sdf = max(inner_sdf, outer_sdf);

    if (border_sdf < antialiasThreshold) {
        let blended_border = over(color, borderColor);
        color = lerp(color, blended_border, saturate(0.5 - inner_sdf / pixelSize));
    }

    return float4(color.rgb, color.a * saturate(0.5 - outer_sdf / pixelSize));
}

float sdf_rect_impl(float2 corner_center_to_point, float corner_radius) {
    if (corner_radius == 0.0) {
        return max(corner_center_to_point.x, corner_center_to_point.y);
    } else {
        let signed_distance_to_inset_quad =
            length(max(float2(0.0, 0.0), corner_center_to_point))
            + min(0.0, max(corner_center_to_point.x, corner_center_to_point.y));

        return signed_distance_to_inset_quad - corner_radius;
    }
}
