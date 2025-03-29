#ifndef _SGE_RENDERER_SHADERS_HPP_
#define _SGE_RENDERER_SHADERS_HPP_

static const char D3D11_FONT[1929] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
    float2 position : Position;
    
    float3 i_color : I_Color;
    float2 i_position : I_Position;
    float2 i_size : I_Size;
    float2 i_tex_size : I_TexSize;
    float2 i_uv : I_UV;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    nointerpolation float3 color : Color;
    nointerpolation float2 tex_size : TexSize;
    float2 uv : UV;
};

static const uint FLAG_UI = 1 << 0;

VSOutput VS(VSInput inp)
{
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? u_screen_projection : u_view_projection;

    const float2 position = inp.i_position + inp.position * inp.i_size;
    const float2 uv = inp.i_uv + inp.position * inp.i_tex_size;

    VSOutput outp;
    outp.color = inp.i_color;
    outp.uv = uv;
    outp.tex_size = inp.i_tex_size;
    outp.position = mul(mvp, float4(position, 0.0, 1.0));
    outp.position.z = 1.0f;

	return outp;
}

Texture2D Texture : register(t3);
SamplerState Sampler : register(s4);

static const float GLYPH_CENTER = 0.5;

float4 PS(VSOutput inp) : SV_Target
{
    const float outline = 0.2;

    const float dist = Texture.Sample(Sampler, inp.uv).r;
    const float width = fwidth(dist);
    const float alpha = smoothstep(GLYPH_CENTER - outline - width, GLYPH_CENTER - outline + width, abs(dist));
    // float4 color = float4(inp.color, alpha);

    const float mu = smoothstep(0.5-width, 0.5+width, abs(dist));
    const float3 rgb = lerp(float3(0.0, 0.0, 0.0), inp.color, mu);
    const float4 color = float4(rgb, alpha);

    clip(color.a - 0.05f);

    return color;
};

)";

static const char D3D11_NINEPATCH[5061] = R"(cbuffer GlobalUniformBuffer : register( b2 )
{
    float4x4 u_screen_projection;
    float4x4 u_view_projection;
    float4x4 u_nozoom_view_projection;
    float4x4 u_nozoom_projection;
    float4x4 u_transform_matrix;
    float4x4 u_inv_view_proj;
    float2 u_camera_position;
    float2 u_window_size;
};

struct VSInput
{
    float2 position : Position;

    float3 i_position : I_Position;
    float4 i_rotation : I_Rotation;
    float2 i_size : I_Size;
    float2 i_offset : I_Offset;
    float2 i_source_size: I_SourceSize;
    float2 i_output_size: I_OutputSize;
    uint4  i_margin : I_Margin;
    float4 i_uv_offset_scale : I_UvOffsetScale;
    float4 i_color : I_Color;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    nointerpolation float4 color : Color;
    nointerpolation uint4  margin : Margin;
    nointerpolation float2 source_size: SourceSize;
    nointerpolation float2 output_size: OutputSize;
    float2 uv : UV;
};

static const uint FLAG_UI = 1 << 0;

VSOutput VS(VSInput inp)
{
    const float qxx = inp.i_rotation.x * inp.i_rotation.x;
    const float qyy = inp.i_rotation.y * inp.i_rotation.y;
    const float qzz = inp.i_rotation.z * inp.i_rotation.z;
    const float qxz = inp.i_rotation.x * inp.i_rotation.z;
    const float qxy = inp.i_rotation.x * inp.i_rotation.y;
    const float qyz = inp.i_rotation.y * inp.i_rotation.z;
    const float qwx = inp.i_rotation.w * inp.i_rotation.x;
    const float qwy = inp.i_rotation.w * inp.i_rotation.y;
    const float qwz = inp.i_rotation.w * inp.i_rotation.z;

    const float4x4 rotation_matrix = float4x4(
        float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy - qwz)      , 2.0 * (qxz + qwy)      , 0.0),
        float4(2.0 * (qxy + qwz)      , 1.0 - 2.0 * (qxx + qzz), 2.0 * (qyz - qwx)      , 0.0),
        float4(2.0 * (qxz - qwy)      , 2.0 * (qyz + qwx)      , 1.0 - 2.0 * (qxx + qyy), 0.0),
        float4(0.0                    , 0.0                    , 0.0                    , 1.0)
    );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, inp.i_position.x),
        float4(0.0, 1.0, 0.0, inp.i_position.y),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    transform = mul(transform, rotation_matrix);

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

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = mul(u_screen_projection, transform);
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VSOutput outp;
    outp.position = mul(mvp, float4(inp.position, 0.0, 1.0));
    outp.position.z = 1.0f;
    outp.uv = inp.position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.margin = inp.i_margin;
    outp.source_size = inp.i_source_size;
    outp.output_size = inp.i_output_size;

	return outp;
}

Texture2D Texture : register(t3);
SamplerState Sampler : register(s4);

float map(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
} 

float process_axis(float coord, float2 source_margin, float2 out_margin) {
    if (coord < out_margin.x) {
        return map(coord, 0.0f, out_margin.x, 0.0f, source_margin.x);
    }
    if (coord < 1.0f - out_margin.y) {
        return map(coord, out_margin.x, 1.0f - out_margin.y, source_margin.x, 1.0f - source_margin.y);
    }
    return map(coord, 1.0f - out_margin.y, 1.0f, 1.0f - source_margin.y, 1.0f);
}

float4 PS(VSOutput inp) : SV_Target
{
    const float2 horizontal_margin = inp.margin.xy;
    const float2 vertical_margin = inp.margin.zw;

    const float2 new_uv = float2(
        process_axis(inp.uv.x,
            horizontal_margin / inp.source_size.xx,
            horizontal_margin / inp.output_size.xx
        ),
        process_axis(inp.uv.y,
            vertical_margin / inp.source_size.yy,
            vertical_margin / inp.output_size.yy
        )
    );

    const float4 color = Texture.Sample(Sampler, new_uv) * inp.color;

    clip(color.a - 0.5);

    return color;
};

)";

static const char D3D11_SHAPE[7450] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
    float4 i_border_radius: I_BorderRadius;
    float i_border_thickness : I_BorderThickness;
    uint i_shape : I_Shape;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
    nointerpolation float2 size : Size;
    nointerpolation float4 color : Color;
    nointerpolation float4 border_color : BorderColor;
    nointerpolation float4 border_radius : BorderRadius;
    nointerpolation float border_thickness : BorderThickness;
    nointerpolation uint shape : Shape;
};

static const uint FLAG_UI = 1 << 0;

static const uint SHAPE_CIRCLE = 1;
static const uint SHAPE_ARC = 2;

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

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = mul(is_ui ? u_screen_projection : u_view_projection, transform);
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

static const float PI = 3.1415926535;
static const float TAU = 6.283185307179586;

float mod(float x, float y) {
    return x - y * floor(x/y);
}

float arc_sdf(in float2 p, in float a0, in float a1, in float r )
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

float4 PS(VSOutput inp) : SV_Target
{
    if (inp.shape == SHAPE_CIRCLE) {
        return circle(inp.uv, inp.color, inp.border_color, inp.border_thickness);
    } else if (inp.shape == SHAPE_ARC) {
        float start_angle = inp.border_radius.x;
        float end_angle = inp.border_radius.y;

        float thickness = 0.5 - inp.border_thickness / inp.size.y;

        float2 p = ((float2(inp.uv.x, 1.0 - inp.uv.y) * 2.0 - 1.0) * inp.size) / inp.size.y;

        float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
        float aa = length(float2(ddx(d), ddy(d)));

        float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);

        return float4(inp.color.rgb, min(inp.color.a, alpha));
    }

    float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));

    if (radius > 0.0) {
        float d = rounded_box_sdf(inp.uv, inp.size, inp.border_radius);
        float aa = length(float2(ddx(d), ddy(d)));

        float smoothed_alpha = 1.0 - smoothstep(0.0-aa, 0.0, d);
        float border_alpha = 1.0 - smoothstep(inp.border_thickness - aa, inp.border_thickness, abs(d));

        float4 quad_color = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
        float4 quad_color_with_border = lerp(quad_color, inp.border_color, min(inp.border_color.a, min(border_alpha, smoothed_alpha)));

        float4 result = float4(quad_color_with_border.rgb, lerp(0.0, quad_color_with_border.a, smoothed_alpha));

        if (result.a < 0.01) discard;
        
        return result;
    }

    return inp.color;
};)";

static const char D3D11_SPRITE[5296] = R"(cbuffer GlobalUniformBuffer : register( b2 )
{
    float4x4 u_screen_projection;
    float4x4 u_view_projection;
    float4x4 u_nozoom_view_projection;
    float4x4 u_nozoom_projection;
    float4x4 u_transform_matrix;
    float4x4 u_inv_view_proj;
    float2 u_camera_position;
    float2 u_window_size;
};

struct VSInput
{
    float2 position : Position;

    float3 i_position : I_Position;
    float4 i_rotation : I_Rotation;
    float2 i_size : I_Size;
    float2 i_offset : I_Offset;
    float4 i_uv_offset_scale : I_UvOffsetScale;
    float4 i_color : I_Color;
    float4 i_outline_color : I_OutlineColor;
    float i_outline_thickness : I_OutlineThickness;
    uint i_flags : I_Flags;
};

struct VSOutput
{
    float4 position : SV_Position;
    nointerpolation float4 color : Color;
    nointerpolation float4 outline_color : OutlineColor;
    float2 uv : UV;
    nointerpolation float outline_thickness : OutlineThickness;
};

static const uint FLAG_UI = 1 << 0;
static const int FLAG_IGNORE_CAMERA_ZOOM = 1 << 1;

VSOutput VS(VSInput inp)
{
    const float qxx = inp.i_rotation.x * inp.i_rotation.x;
    const float qyy = inp.i_rotation.y * inp.i_rotation.y;
    const float qzz = inp.i_rotation.z * inp.i_rotation.z;
    const float qxz = inp.i_rotation.x * inp.i_rotation.z;
    const float qxy = inp.i_rotation.x * inp.i_rotation.y;
    const float qyz = inp.i_rotation.y * inp.i_rotation.z;
    const float qwx = inp.i_rotation.w * inp.i_rotation.x;
    const float qwy = inp.i_rotation.w * inp.i_rotation.y;
    const float qwz = inp.i_rotation.w * inp.i_rotation.z;

    const float4x4 rotation_matrix = float4x4(
        float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy - qwz)      , 2.0 * (qxz + qwy)      , 0.0),
        float4(2.0 * (qxy + qwz)      , 1.0 - 2.0 * (qxx + qzz), 2.0 * (qyz - qwx)      , 0.0),
        float4(2.0 * (qxz - qwy)      , 2.0 * (qyz + qwx)      , 1.0 - 2.0 * (qxx + qyy), 0.0),
        float4(0.0                    , 0.0                    , 0.0                    , 1.0)
    );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, inp.i_position.x),
        float4(0.0, 1.0, 0.0, inp.i_position.y),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    transform = mul(transform, rotation_matrix);

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

    const int flags = inp.i_flags;
    const bool ignore_camera_zoom = (flags & FLAG_IGNORE_CAMERA_ZOOM) == FLAG_IGNORE_CAMERA_ZOOM;
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = mul(is_ui ? u_screen_projection : ignore_camera_zoom ? u_nozoom_view_projection : u_view_projection, transform);
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VSOutput outp;
    outp.position = mul(mvp, float4(inp.position, 0.0, 1.0));
    outp.position.z = inp.i_position.z;
    outp.uv = inp.position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.outline_color = inp.i_outline_color;
    outp.outline_thickness = inp.i_outline_thickness;

	return outp;
}

Texture2D Texture : register(t3);
SamplerState Sampler : register(s4);

float4 PS(VSOutput inp) : SV_Target
{
    float4 color = inp.color;

    if (inp.outline_thickness > 0.0) {
        float outline = Texture.Sample(Sampler, inp.uv + float2(inp.outline_thickness, 0.0)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(-inp.outline_thickness, 0.0)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(0.0, inp.outline_thickness)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(0.0, -inp.outline_thickness)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(inp.outline_thickness, -inp.outline_thickness)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(-inp.outline_thickness, inp.outline_thickness)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(inp.outline_thickness, inp.outline_thickness)).a;
        outline += Texture.Sample(Sampler, inp.uv + float2(-inp.outline_thickness, -inp.outline_thickness)).a;
        outline = min(outline, 1.0);
        float4 c = Texture.Sample(Sampler, inp.uv);
        color = lerp(c, inp.outline_color, outline);
    } else {
        color = Texture.Sample(Sampler, inp.uv) * inp.color;
    }

    clip(color.a - 0.5);

    return color;
};

)";

static const char METAL_FONT[1612] = R"(#include <metal_stdlib>

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
    float max_depth;
    float max_world_depth;
};

struct VertexIn
{
    float2 position   [[attribute(0)]];

    float3 i_color    [[attribute(1)]];
    float3 i_position [[attribute(2)]];
    float2 i_size     [[attribute(3)]];
    float2 i_tex_size [[attribute(4)]];
    float2 i_uv       [[attribute(5)]];
    int    i_is_ui    [[attribute(6)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 color [[flat]];
    float2 uv;
};

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const float4x4 mvp = inp.i_is_ui > 0 ? constants.screen_projection : constants.view_projection;

    const float2 position = inp.i_position.xy + inp.position * inp.i_size;
    const float2 uv = inp.i_uv + inp.position * inp.i_tex_size;

	VertexOut outp;
    outp.color = inp.i_color;
    outp.uv = uv;
    outp.position = mvp * float4(position, 0.0, 1.0);
    outp.position.z = inp.i_position.z / constants.max_depth;

    return outp;
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    const float4 color = float4(inp.color, texture.sample(texture_sampler, inp.uv).r);

    if (color.a < 0.05) discard_fragment();

    return color;
})";

static const char METAL_SPRITE[4971] = R"(#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 transform_matrix;
    float2 camera_position;
    float2 window_size;
    float max_depth;
};

struct VertexIn
{
    float2 position            [[attribute(0)]];

    float3 i_position          [[attribute(1)]];
    float4 i_rotation          [[attribute(2)]];
    float2 i_size              [[attribute(3)]];
    float2 i_offset            [[attribute(4)]];
    float4 i_uv_offset_scale   [[attribute(5)]];
    float4 i_color             [[attribute(6)]];
    float4 i_outline_color     [[attribute(7)]];
    float  i_outline_thickness [[attribute(8)]];
    int    i_flags             [[attribute(9)]];
};

struct VertexOut
{
    float4 position          [[position]];
    float2 uv;
    float4 color             [[flat]];
    float4 outline_color     [[flat]];
    float  outline_thickness [[flat]];
    bool   has_texture       [[flat]];
};

constant constexpr int HAS_TEXTURE_FLAG = 1 << 0;
constant constexpr int IS_UI_FLAG = 1 << 1;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const float qxx = inp.i_rotation.x * inp.i_rotation.x;
    const float qyy = inp.i_rotation.y * inp.i_rotation.y;
    const float qzz = inp.i_rotation.z * inp.i_rotation.z;
    const float qxz = inp.i_rotation.x * inp.i_rotation.z;
    const float qxy = inp.i_rotation.x * inp.i_rotation.y;
    const float qyz = inp.i_rotation.y * inp.i_rotation.z;
    const float qwx = inp.i_rotation.w * inp.i_rotation.x;
    const float qwy = inp.i_rotation.w * inp.i_rotation.y;
    const float qwz = inp.i_rotation.w * inp.i_rotation.z;

    const float4x4 rotation_matrix = float4x4(
        float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
        float4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
        float4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(inp.i_position.x, inp.i_position.y, 0.0, 1.0)
    );

    transform = transform * rotation_matrix;

    const float2 offset = -inp.i_offset * inp.i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * inp.i_size[0];
    transform[1] = transform[1] * inp.i_size[1];

    const int flags = inp.i_flags;
    const bool is_ui = (flags & IS_UI_FLAG) == IS_UI_FLAG;
    const bool has_texture = (flags & HAS_TEXTURE_FLAG) == HAS_TEXTURE_FLAG;

    const float4x4 mvp = is_ui ? constants.screen_projection * transform : constants.view_projection * transform;
    const float4 uv_offset_scale = inp.i_uv_offset_scale;
    const float2 position = inp.position;
    const float max_depth = constants.max_depth;
    const float order = inp.i_position.z / max_depth;

    VertexOut outp;
    outp.position = mvp * float4(position, 0.0, 1.0);
    outp.position.z = order;
    outp.uv = position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.outline_color = inp.i_outline_color;
    outp.outline_thickness = inp.i_outline_thickness;
    outp.has_texture = has_texture;

    return outp;
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    float4 color = inp.color;

    if (inp.has_texture > 0) {
        if (inp.outline_thickness > 0.0) {
            float outline = texture.sample(texture_sampler, inp.uv + float2(inp.outline_thickness, 0.0)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(-inp.outline_thickness, 0.0)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(0.0, inp.outline_thickness)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(0.0, -inp.outline_thickness)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(inp.outline_thickness, -inp.outline_thickness)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(-inp.outline_thickness, inp.outline_thickness)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(inp.outline_thickness, inp.outline_thickness)).a;
            outline += texture.sample(texture_sampler, inp.uv + float2(-inp.outline_thickness, -inp.outline_thickness)).a;
            outline = min(outline, 1.0);
            float4 c = texture.sample(texture_sampler, inp.uv);
            color = mix(c, inp.outline_color, outline);
        } else {
            color = texture.sample(texture_sampler, inp.uv) * inp.color;
        }
    }

    if (color.a < 0.5) discard_fragment();

    return color;
})";

static const char GL_FONT_FRAG[265] = R"(#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;
flat in vec3 v_color;

uniform sampler2D u_texture;

void main() {
    vec4 color = vec4(v_color, texture(u_texture, v_uv).r);

    if (color.a <= 0.05) discard;

    frag_color = color;
})";

static const char GL_FONT_VERT[887] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec3 i_position;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_tex_size;
layout(location = 5) in vec2 i_uv;
layout(location = 6) in int i_ui;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 transform_matrix;
    vec2 camera_position;
    vec2 window_size;
    float max_depth;
} global_ubo;

out vec2 v_uv;
flat out vec3 v_color;

void main() {
    mat4 mvp = i_ui > 0 ? global_ubo.screen_projection : global_ubo.view_projection;

    vec2 position = i_position.xy + a_position * i_size;
    vec2 uv = i_uv + a_position * i_tex_size;

    v_uv = uv;
    v_color = i_color;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    gl_Position.z = i_position.z / global_ubo.max_depth;
})";

static const char GL_SPRITE_FRAG[1349] = R"(#version 330 core

in vec2 v_uv;
flat in vec4 v_color;
flat in vec4 v_outline_color;
flat in float v_outline_thickness;
flat in int v_has_texture;

out vec4 frag_color;

uniform sampler2D u_texture;

void main() {
    vec4 color = v_color;

    if (v_has_texture > 0) {
        if (v_outline_thickness > 0.0) {
            float outline = texture(u_texture, v_uv + vec2(v_outline_thickness, 0.0)).a;
            outline += texture(u_texture, v_uv + vec2(-v_outline_thickness, 0.0)).a;
            outline += texture(u_texture, v_uv + vec2(0.0, v_outline_thickness)).a;
            outline += texture(u_texture, v_uv + vec2(0.0, -v_outline_thickness)).a;
            outline += texture(u_texture, v_uv + vec2(v_outline_thickness, -v_outline_thickness)).a;
            outline += texture(u_texture, v_uv + vec2(-v_outline_thickness, v_outline_thickness)).a;
            outline += texture(u_texture, v_uv + vec2(v_outline_thickness, v_outline_thickness)).a;
            outline += texture(u_texture, v_uv + vec2(-v_outline_thickness, -v_outline_thickness)).a;
            outline = min(outline, 1.0);
            vec4 c = texture(u_texture, v_uv);
            color = mix(c, v_outline_color, outline);
        } else {
            color = texture(u_texture, v_uv) * v_color;
        }
    }

    if (color.a < 0.5) discard;

    frag_color = color;
})";

static const char GL_SPRITE_VERT[2706] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec4 i_rotation;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_offset;
layout(location = 5) in vec4 i_uv_offset_scale;
layout(location = 6) in vec4 i_color;
layout(location = 7) in vec4 i_outline_color;
layout(location = 8) in float i_outline_thickness;
layout(location = 9) in int i_flags;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 transform_matrix;
    vec2 camera_position;
    vec2 window_size;
    float max_depth;
} global_ubo;

out vec2 v_uv;
flat out vec4 v_color;
flat out vec4 v_outline_color;
flat out float v_outline_thickness;
flat out int v_has_texture;

const int HAS_TEXTURE_FLAG = 1 << 0;
const int IS_UI_FLAG = 1 << 1;

void main() {
    float qxx = i_rotation.x * i_rotation.x;
    float qyy = i_rotation.y * i_rotation.y;
    float qzz = i_rotation.z * i_rotation.z;
    float qxz = i_rotation.x * i_rotation.z;
    float qxy = i_rotation.x * i_rotation.y;
    float qyz = i_rotation.y * i_rotation.z;
    float qwx = i_rotation.w * i_rotation.x;
    float qwy = i_rotation.w * i_rotation.y;
    float qwz = i_rotation.w * i_rotation.z;

    mat4 rotation_matrix = mat4(
        vec4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
        vec4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
        vec4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    mat4 transform = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(i_position.x, i_position.y, 0.0, 1.0)
    );

    transform *= rotation_matrix;

    vec2 offset = -i_offset * i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * i_size[0];
    transform[1] = transform[1] * i_size[1];

    bool is_ui = (i_flags & IS_UI_FLAG) == IS_UI_FLAG;
    bool has_texture = (i_flags & HAS_TEXTURE_FLAG) == HAS_TEXTURE_FLAG;

    mat4 mvp = (is_ui ? global_ubo.screen_projection : global_ubo.view_projection) * transform;
    
    float max_depth = global_ubo.max_depth;
    float order = i_position.z / max_depth;

    v_uv = a_position * i_uv_offset_scale.zw + i_uv_offset_scale.xy;
    v_color = i_color;
    v_outline_color = i_outline_color;
    v_outline_thickness = i_outline_thickness;
    v_has_texture = has_texture ? 1 : 0;
    
    gl_Position = mvp * vec4(a_position, 0, 1);
    gl_Position.z = order;
})";

#endif