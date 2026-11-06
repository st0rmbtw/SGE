#include "common.slang"
#include "sdf.slang"

[vk::binding(2)]
cbuffer GlobalUniformBuffer : register( b2 )
{
    GlobalUniforms uniforms;
};

struct VSInput
{
    float2 position : Position;

    float4 i_color : I_Color;
    float4 i_border_color : I_BorderColor;
    float4 i_corner_radii: I_CornerRadii;
    float3 i_position : I_Position;
    float2 i_size : I_Size;
    float2 i_offset : I_Offset;
    float i_border_thickness : I_BorderThickness;
    uint i_shape : I_Shape;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
    float2 p  : Point;
    nointerpolation float2 size : Size;
    nointerpolation float4 color : Color;
    nointerpolation float4 borderColor : BorderColor;
    nointerpolation float4 cornerRadii : CornerRadii;
    nointerpolation float borderThickness : BorderThickness;
    nointerpolation uint shape : Shape;
};

static const uint FLAG_UI = 1 << 0;

static const uint SHAPE_CIRCLE = 1;
static const uint SHAPE_ARC = 2;

[shader("vertex")]
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

    var transform = float4x4(
        float4(1.0, 0.0, 0.0, inp.i_position.x),
        float4(0.0, 1.0, 0.0, inp.i_position.y),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    // transform = mul(transform, rotation_matrix);

    let offset = -inp.i_offset * inp.i_size;

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

    let is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    let mvp = mul(is_ui ? uniforms.screen_projection : uniforms.view_projection, transform);

    VSOutput outp;
    outp.position = mul(mvp, float4(inp.position, 0.0, 1.0));
    outp.position.z = inp.i_position.z;
    outp.uv = inp.position;
    outp.p = (inp.position - 0.5) * inp.i_size;
    outp.size = inp.i_size;
    outp.color = inp.i_color;
    outp.borderColor = inp.i_border_color;
    outp.borderThickness = inp.i_border_thickness;
    outp.cornerRadii = inp.i_corner_radii;
    outp.shape = inp.i_shape;

	return outp;
}

float antialias_circle(float d) {
    float afwidth = fwidth(d) * 1.3;
    return 1.0 - smoothstep(0.0, afwidth, d);
}

[shader("fragment")]
float4 PS(VSOutput inp) : SV_Target
{
    var result = inp.color;

    if (inp.shape == SHAPE_CIRCLE) {
        let radius = 0.5 - 1.0 / inp.size.x;
        let external_distance = sdf_circle(inp.uv - 0.5, radius);
        let alpha = antialias_circle(external_distance);

        if (inp.borderThickness > 0.0) {
            let internal_distance = sdf_circle(inp.uv - 0.5, radius - inp.borderThickness / inp.size.x);

            let border_distance = max(external_distance, -internal_distance);

            let border_alpha = antialias_circle(border_distance);

            let color = float4(inp.color.rgb, min(inp.color.a, alpha));
            let color_with_border = lerp(color, inp.borderColor, min(inp.borderColor.a, min(border_alpha, alpha)));

            result = internal_distance > 0.0 && border_alpha < 1.0
                ? lerp(float4(0.0, 0.0, 0.0, 0.0), inp.borderColor, border_alpha)
                : color_with_border;
        } else {
            result = float4(inp.color.rgb, min(inp.color.a, alpha));
        }
    } else if (inp.shape == SHAPE_ARC) {
        let start_angle = inp.cornerRadii.x;
        let end_angle = inp.cornerRadii.y;

        let thickness = 0.5 - inp.borderThickness / inp.size.y;

        let p = ((float2(inp.uv.x, 1.0 - inp.uv.y) * 2.0 - 1.0) * inp.size) / inp.size.y;

        let d = sdf_arc(p, start_angle, end_angle, 1.0 - thickness);
        let aa = length(float2(ddx(d), ddy(d)));

        let alpha = 1.0 - smoothstep(thickness - aa, thickness, d);

        return float4(inp.color.rgb, min(inp.color.a, alpha));
    } else {
        return sdf_rect(inp.p, inp.size, inp.color, inp.cornerRadii, inp.borderThickness, inp.borderColor);
    }

    if (result.a <= 0.01f) discard;

    return result;
}
