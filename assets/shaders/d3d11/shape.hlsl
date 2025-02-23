cbuffer GlobalUniformBuffer : register( b2 )
{
    float4x4 u_screen_projection;
    float4x4 u_view_projection;
    float4x4 u_transform_matrix;
    float2 u_camera_position;
    float2 u_window_size;
};

struct VSInput
{
    float2 position : Position;

    float3 i_position : I_Position;
    float2 i_size : I_Size;
    float2 i_offset : I_Offset;
    float4 i_color : I_Color;
    float4 i_border_color : I_BorderColor;
    float i_border_thickness : I_BorderThickness;
    float i_border_radius: I_BorderRadius;
    uint i_shape : I_Shape;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
    nointerpolation float2 size : Size;
    nointerpolation float4 color : Color;
    nointerpolation float4 border_color : BorderColor;
    nointerpolation float border_thickness : BorderThickness;
    nointerpolation float border_radius : BorderRadius;
    nointerpolation uint shape : Shape;
};

static const uint SHAPE_CIRCLE = 1;

VSOutput VS(VSInput inp)
{
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
    //     float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy - qwz)      , 2.0 * (qxz + qwy)      , 0.0),
    //     float4(2.0 * (qxy + qwz)      , 1.0 - 2.0 * (qxx + qzz), 2.0 * (qyz - qwx)      , 0.0),
    //     float4(2.0 * (qxz - qwy)      , 2.0 * (qyz + qwx)      , 1.0 - 2.0 * (qxx + qyy), 0.0),
    //     float4(0.0                    , 0.0                    , 0.0                    , 1.0)
    // );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, inp.i_position.x),
        float4(0.0, 1.0, 0.0, inp.i_position.y),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    // transform = mul(transform, rotation_matrix);

    const float2 offset = -inp.i_offset * inp.i_size;

    // translate
    transform[0][3] = transform[0][0] * offset[0] + transform[0][1] * offset[1] + transform[0][2] * 0.0 + transform[0][3];
    transform[1][3] = transform[1][0] * offset[0] + transform[1][1] * offset[1] + transform[1][2] * 0.0 + transform[1][3];
    transform[2][3] = transform[2][0] * offset[0] + transform[2][1] * offset[1] + transform[2][2] * 0.0 + transform[2][3];
    transform[3][3] = transform[3][0] * offset[0] + transform[3][1] * offset[1] + transform[3][2] * 0.0 + transform[3][3];

    //scale
    transform[0][0] = transform[0][0] * inp.i_size[0];
    transform[1][0] = transform[1][0] * inp.i_size[0];
    transform[2][0] = transform[2][0] * inp.i_size[0];
    transform[3][0] = transform[3][0] * inp.i_size[0];

    transform[0][1] = transform[0][1] * inp.i_size[1];
    transform[1][1] = transform[1][1] * inp.i_size[1];
    transform[2][1] = transform[2][1] * inp.i_size[1];
    transform[3][1] = transform[3][1] * inp.i_size[1];

    const float4x4 mvp = mul(u_view_projection, transform);
    const float2 position = inp.position;

    VSOutput outp;
    outp.position = mul(mvp, float4(position, 0.0, 1.0));
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

static const float CIRCLE_AA = 0.001;

float4 circle(in float2 st, in float4 color, in float4 border_color, float border_thickness) {
    float2 dist = st - float2(0.5, 0.5);
    float d = dot(dist, dist);
    
    float border_cr = 0.5 - border_thickness * 0.5;
    float2 border_weight = float2(border_cr*border_cr+border_cr*CIRCLE_AA,border_cr*border_cr-border_cr*CIRCLE_AA);

    float cr = 0.5;
    float2 weight = float2(cr*cr+cr*CIRCLE_AA,cr*cr-cr*CIRCLE_AA);

    float t1 = 1.0 - clamp((d-border_weight.y)/(border_weight.x-border_weight.y),0.0,1.0);
    float t2 = 1.0 - clamp((d-weight.y)/(weight.x-weight.y),0.0,1.0);

    return float4(lerp(border_color.rgb, color.rgb, t1), t2);
}

float rounded_box_sdf(float2 uv, float2 size, float radius, float aa) {
    float2 center = size * (uv - 0.5);
    float2 dist = abs(center) - size * 0.5 + radius;
    return min(max(dist.x,dist.y),0.0) + length(max(dist, 0.0)) - radius + aa;
}

float4 PS(VSOutput inp) : SV_Target
{
    if (inp.shape == SHAPE_CIRCLE) {
        return circle(inp.uv, inp.color, inp.border_color, inp.border_thickness);
    }

    if (inp.border_radius > 0.0) {
        float aa = 1.0;
        float d = rounded_box_sdf(inp.uv, inp.size, inp.border_radius, aa);

        float smoothed_alpha = 1.0 - smoothstep(0.0, aa * 2.0, d);
        float border_alpha = 1.0 - smoothstep(inp.border_thickness - aa, inp.border_thickness, abs(d));

        float4 quad_color = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
        float4 quad_color_with_border = lerp(quad_color, inp.border_color, min(inp.border_color.a, min(border_alpha, smoothed_alpha)));

        float4 result = float4(quad_color_with_border.rgb, lerp(0.0, quad_color_with_border.a, smoothed_alpha));

        if (result.a < 0.05) discard;
        
        return result;
    }

    return inp.color;
};