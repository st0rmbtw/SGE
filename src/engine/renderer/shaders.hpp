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

static const char D3D11_NINEPATCH[5089] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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

    const float4x4 mvp = mul(is_ui ? u_screen_projection : u_view_projection, transform);
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

static const char METAL_FONT[2114] = R"(#include <metal_stdlib>

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
    float2 position   [[attribute(0)]];

    float3 i_color    [[attribute(1)]];
    float2 i_position [[attribute(2)]];
    float2 i_size     [[attribute(3)]];
    float2 i_tex_size [[attribute(4)]];
    float2 i_uv       [[attribute(5)]];
    uint   i_flags    [[attribute(6)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 color [[flat]];
    float2 tex_size [[flat]];
    float2 uv;
};

constant constexpr uint FLAG_UI = 1 << 0;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = is_ui ? constants.screen_projection : constants.view_projection;

    const float2 position = inp.i_position + inp.position * inp.i_size;
    const float2 uv = inp.i_uv + inp.position * inp.i_tex_size;

	VertexOut outp;
    outp.color = inp.i_color;
    outp.uv = uv;
    outp.tex_size = inp.i_tex_size;
    outp.position = mvp * float4(position, 0.0, 1.0);
    outp.position.z = 1.0;

    return outp;
}

constant const float GLYPH_CENTER = 0.5;

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
    const float outline = 0.2;

    const float dist = texture.sample(texture_sampler, inp.uv).r;
    const float width = fwidth(dist);
    const float alpha = smoothstep(GLYPH_CENTER - outline - width, GLYPH_CENTER - outline + width, abs(dist));
    // float4 color = float4(inp.color, alpha);

    const float mu = smoothstep(0.5-width, 0.5+width, abs(dist));
    const float3 rgb = lerp(float3(0.0, 0.0, 0.0), inp.color, mu);
    const float4 color = float4(rgb, alpha);

    if (color.a < 0.05) discard_fragment();

    return color;
})";

static const char METAL_NINEPATCH[4434] = R"(#include <metal_stdlib>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 nozoom_view_projection;
    float4x4 nozoom_projection;
    float4x4 transform_matrix;
    float4x4 inv_view_proj;
    float2 camera_position;
    float2 window_size;
};

struct VertexIn
{
    float2 position [[attribute(0)]];

    float3 i_position [[attribute(1)]];
    float4 i_rotation [[attribute(2)]];
    float2 i_size [[attribute(3)]];
    float2 i_offset [[attribute(4)]];
    float2 i_source_size[[attribute(5)]];
    float2 i_output_size[[attribute(6)]];
    uint4  i_margin [[attribute(7)]];
    float4 i_uv_offset_scale [[attribute(8)]];
    float4 i_color [[attribute(9)]];
    uint i_flags [[attribute(10)]];
};

struct VertexOut
{
    float4 position [[position]];

    float4 color;
    uint4  margin [[flat]];
    float2 source_size [[flat]];
    float2 output_size [[flat]];
    float2 uv;
};

constant constexpr uint FLAG_UI = 1 << 0;

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

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = (is_ui ? constants.screen_projection : constants.view_projection) * transform;
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VertexOut outp;
    outp.position = mvp * float4(inp.position, 0.0, 1.0);
    outp.position.z = 1.0;
    outp.uv = inp.position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.margin = inp.i_margin;
    outp.source_size = inp.i_source_size;
    outp.output_size = inp.i_output_size;

    return outp;
}

constant constexpr float GLYPH_CENTER = 0.5;

float map(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
} 

float process_axis(float coord, float2 source_margin, float2 out_margin) {
    if (coord < out_margin.x) {
        return map(coord, 0.0f, out_margin.x, 0.0f, source_margin.x);
    }
    if (coord < 1.0 - out_margin.y) {
        return map(coord, out_margin.x, 1.0 - out_margin.y, source_margin.x, 1.0 - source_margin.y);
    }
    return map(coord, 1.0 - out_margin.y, 1.0, 1.0 - source_margin.y, 1.0);
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
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

    const float4 color = texture.sample(texture_sampler, new_uv) * inp.color;

    if (color.a < 0.05) discard_fragment();

    return color;
})";

static const char METAL_SHAPE[4631] = R"(#include <metal_stdlib>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 nozoom_view_projection;
    float4x4 nozoom_projection;
    float4x4 transform_matrix;
    float4x4 inv_view_proj;
    float2 camera_position;
    float2 window_size;
};

struct VertexIn
{
    float2 position [[attribute(0)]];

    float3 i_position [[attribute(1)]];
    float2 i_size [[attribute(2)]];
    float2 i_offset [[attribute(3)]];
    float4 i_color [[attribute(4)]];
    float4 i_border_color [[attribute(5)]];
    float4 i_border_radius[[attribute(6)]];
    float i_border_thickness [[attribute(7)]];
    uint i_shape [[attribute(8)]];
    uint i_flags [[attribute(9)]];
};

struct VertexOut
{
    float4 position [[position]];

    float2 uv;
    float2 size [[flat]];
    float4 color [[flat]];
    float4 border_color [[flat]];
    float4 border_radius [[flat]];
    float border_thickness [[flat]];
    uint shape [[flat]];
};

constant const uint FLAG_UI = 1 << 0;

constant const uint SHAPE_CIRCLE = 1;
constant const uint SHAPE_ARC = 2;

vertex VertexOut VS(
    VertexIn inp [[stage_in]],
    constant Constants& constants [[buffer(2)]]
) {
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
    //     float4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
    //     float4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
    //     float4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
    //     float4(0.0, 0.0, 0.0, 1.0)
    // );

    float4x4 transform = float4x4(
        float4(1.0, 0.0, 0.0, 0.0),
        float4(0.0, 1.0, 0.0, 0.0),
        float4(0.0, 0.0, 1.0, 0.0),
        float4(inp.i_position.x, inp.i_position.y, 0.0, 1.0)
    );

    // transform = transform * rotation_matrix;

    const float2 offset = -inp.i_offset * inp.i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * inp.i_size[0];
    transform[1] = transform[1] * inp.i_size[1];

    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = (is_ui ? constants.screen_projection : constants.view_projection) * transform;
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VertexOut outp;
    outp.position = mvp * float4(position, 0.0, 1.0);
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

constant constexpr float GLYPH_CENTER = 0.5;

float map(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
} 

float process_axis(float coord, float2 source_margin, float2 out_margin) {
    if (coord < out_margin.x) {
        return map(coord, 0.0f, out_margin.x, 0.0f, source_margin.x);
    }
    if (coord < 1.0 - out_margin.y) {
        return map(coord, out_margin.x, 1.0 - out_margin.y, source_margin.x, 1.0 - source_margin.y);
    }
    return map(coord, 1.0 - out_margin.y, 1.0, 1.0 - source_margin.y, 1.0);
}

fragment float4 PS(
    VertexOut inp [[stage_in]],
    texture2d<float> texture [[texture(3)]],
    sampler texture_sampler [[sampler(4)]]
) {
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

    const float4 color = texture.sample(texture_sampler, new_uv) * inp.color;

    if (color.a < 0.05) discard_fragment();

    return color;
})";

static const char METAL_SPRITE[5410] = R"(#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Constants
{
    float4x4 screen_projection;
    float4x4 view_projection;
    float4x4 nozoom_view_projection;
    float4x4 nozoom_projection;
    float4x4 transform_matrix;
    float4x4 inv_view_proj;
    float2 camera_position;
    float2 window_size;
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
    uint   i_flags             [[attribute(9)]];
};

struct VertexOut
{
    float4 position          [[position]];
    float4 color             [[flat]];
    float4 outline_color     [[flat]];
    float2 uv;
    float  outline_thickness [[flat]];
};

constant constexpr int IS_UI_FLAG = 1 << 0;
constant constexpr int FLAG_IGNORE_CAMERA_ZOOM = 1 << 1;

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

    const int flags = inp.i_flags;
    const bool ignore_camera_zoom = (flags & FLAG_IGNORE_CAMERA_ZOOM) == FLAG_IGNORE_CAMERA_ZOOM;
    const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;

    const float4x4 mvp = (is_ui ? constants.screen_projection : ignore_camera_zoom ? constants.nozoom_view_projection : constants.view_projection) * transform;
    const float4 uv_offset_scale = inp.i_uv_offset_scale;

    VertexOut outp;
    outp.position = mvp * float4(inp.position, 0.0, 1.0);
    outp.position.z = inp.i_position.z;
    outp.uv = inp.position * uv_offset_scale.zw + uv_offset_scale.xy;
    outp.color = inp.i_color;
    outp.outline_color = inp.i_outline_color;
    outp.outline_thickness = inp.i_outline_thickness;

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

static const char GL_FONT_FRAG[646] = R"(#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;
flat in vec3 v_color;

uniform sampler2D u_texture;

const float OUTLINE = 0.2;
const float GLYPH_CENTER = 0.5;

void main() {
    float dist = texture(u_texture, v_uv).r;

    float width = fwidth(dist);
    float alpha = smoothstep(GLYPH_CENTER - OUTLINE - width, GLYPH_CENTER - OUTLINE + width, abs(dist));
    // float4 color = float4(inp.color, alpha);

    float mu = smoothstep(0.5-width, 0.5+width, abs(dist));
    vec3 rgb = mix(vec3(0.0, 0.0, 0.0), v_color, mu);
    vec4 color = vec4(rgb, alpha);

    if (color.a <= 0.05) discard;

    frag_color = color;
})";

static const char GL_FONT_VERT[1006] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec3 i_position;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_tex_size;
layout(location = 5) in vec2 i_uv;
layout(location = 6) in uint i_flags;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 nonscale_view_projection;
    mat4 nonscale_projection;
    mat4 transform_matrix;
    mat4 inv_view_proj;
    vec2 camera_position;
    vec2 window_size;
} global_ubo;

out vec2 v_uv;
flat out vec3 v_color;

const uint FLAG_UI = 1u << 0u;

void main() {
    bool is_ui = (i_flags & FLAG_UI) == FLAG_UI;

    mat4 mvp = is_ui ? global_ubo.screen_projection : global_ubo.view_projection;

    vec2 position = i_position.xy + a_position * i_size;
    vec2 uv = i_uv + a_position * i_tex_size;

    v_uv = uv;
    v_color = i_color;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    gl_Position.z = 1.0;
})";

static const char GL_NINEPATCH_FRAG[1274] = R"(#version 330 core

in vec2 v_uv;
flat in vec4 v_color;
flat in uvec4 v_margin;
flat in vec2 v_source_size;
flat in vec2 v_output_size;

out vec4 frag_color;

uniform sampler2D u_texture;

float map(float value, float in_min, float in_max, float out_min, float out_max) {
    return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
} 

float process_axis(float coord, vec2 source_margin, vec2 out_margin) {
    if (coord < out_margin.x) {
        return map(coord, 0.0, out_margin.x, 0.0, source_margin.x);
    }
    if (coord < 1.0 - out_margin.y) {
        return map(coord, out_margin.x, 1.0 - out_margin.y, source_margin.x, 1.0 - source_margin.y);
    }
    return map(coord, 1.0 - out_margin.y, 1.0, 1.0 - source_margin.y, 1.0);
}

void main() {
    vec2 horizontal_margin = v_margin.xy;
    vec2 vertical_margin = v_margin.zw;

    vec2 new_uv = vec2(
        process_axis(v_uv.x,
            horizontal_margin / v_source_size.xx,
            horizontal_margin / v_output_size.xx
        ),
        process_axis(v_uv.y,
            vertical_margin / v_source_size.yy,
            vertical_margin / v_output_size.yy
        )
    );

    vec4 color = texture(u_texture, new_uv) * v_color;

    if (color.a < 0.5) discard;

    frag_color = color;
})";

static const char GL_NINEPATCH_VERT[2755] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec4 i_rotation;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_offset;
layout(location = 5) in vec2 i_source_size;
layout(location = 6) in vec2 i_output_size;
layout(location = 7) in uvec4 i_margin;
layout(location = 8) in vec4 i_uv_offset_scale;
layout(location = 9) in vec4 i_color;
layout(location = 10) in uint i_flags;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 nozoom_view_projection;
    mat4 nozoom_projection;
    mat4 transform_matrix;
    mat4 inv_view_proj;
    vec2 camera_position;
    vec2 window_size;
} global_ubo;

out vec2 v_uv;
flat out vec4 v_color;
flat out uvec4 v_margin;
flat out vec2 v_source_size;
flat out vec2 v_output_size;

const uint IS_UI_FLAG = 1u << 0u;
const uint FLAG_IGNORE_CAMERA_ZOOM = 1u << 1u;

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
    bool ignore_camera_zoom = (i_flags & FLAG_IGNORE_CAMERA_ZOOM) == FLAG_IGNORE_CAMERA_ZOOM;

    mat4 mvp = (is_ui ? global_ubo.screen_projection : ignore_camera_zoom ? global_ubo.nozoom_view_projection : global_ubo.view_projection) * transform;

    v_uv = a_position * i_uv_offset_scale.zw + i_uv_offset_scale.xy;
    v_color = i_color;
    v_margin = i_margin;
    v_source_size = i_source_size;
    v_output_size = i_output_size;
    
    gl_Position = mvp * vec4(a_position, 0, 1);
    gl_Position.z = 1.0;
})";

static const char GL_SHAPE_FRAG[3700] = R"(#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;
flat in vec4 v_color;
flat in vec2 v_size;
flat in vec4 v_border_color;
flat in vec4 v_border_radius;
flat in float v_border_thickness;
flat in uint v_shape;

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

float mod(float x, float y) {
    return x - y * floor(x/y);
}

float arc_sdf(in vec2 p, in float a0, in float a1, in float r )
{
    float a = mod(atan(p.y, p.x), TAU);

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
})";

static const char GL_SHAPE_VERT[2727] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec2 i_size;
layout(location = 3) in vec2 i_offset;
layout(location = 4) in vec4 i_color;
layout(location = 5) in vec4 i_border_color;
layout(location = 6) in vec4 i_border_radius;
layout(location = 7) in float i_border_thickness;
layout(location = 8) in uint i_shape;
layout(location = 9) in uint i_flags;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 transform_matrix;
    vec2 camera_position;
    vec2 window_size;
} global_ubo;

out vec2 v_uv;
flat out vec4 v_color;
flat out vec2 v_size;
flat out vec4 v_border_color;
flat out vec4 v_border_radius;
flat out float v_border_thickness;
flat out uint v_shape;

const uint IS_UI_FLAG = 1u << 0u;

const uint SHAPE_CIRCLE = 1u;
const uint SHAPE_ARC = 2u;

void main() {
    // float qxx = i_rotation.x * i_rotation.x;
    // float qyy = i_rotation.y * i_rotation.y;
    // float qzz = i_rotation.z * i_rotation.z;
    // float qxz = i_rotation.x * i_rotation.z;
    // float qxy = i_rotation.x * i_rotation.y;
    // float qyz = i_rotation.y * i_rotation.z;
    // float qwx = i_rotation.w * i_rotation.x;
    // float qwy = i_rotation.w * i_rotation.y;
    // float qwz = i_rotation.w * i_rotation.z;

    // mat4 rotation_matrix = mat4(
    //     vec4(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy + qwz), 2.0 * (qxz - qwy), 0.0),
    //     vec4(2.0 * (qxy - qwz), 1.0 - 2.0 * (qxx +  qzz), 2.0 * (qyz + qwx), 0.0),
    //     vec4(2.0 * (qxz + qwy), 2.0 * (qyz - qwx), 1.0 - 2.0 * (qxx +  qyy), 0.0),
    //     vec4(0.0, 0.0, 0.0, 1.0)
    // );

    mat4 transform = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(i_position.x, i_position.y, 0.0, 1.0)
    );

    // transform *= rotation_matrix;

    vec2 offset = -i_offset * i_size;

    // translate
    transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];

    // scale
    transform[0] = transform[0] * i_size[0];
    transform[1] = transform[1] * i_size[1];

    bool is_ui = (i_flags & IS_UI_FLAG) == IS_UI_FLAG;
    // bool ignore_camera_zoom = (i_flags & FLAG_IGNORE_CAMERA_ZOOM) == FLAG_IGNORE_CAMERA_ZOOM;

    mat4 mvp = (is_ui ? global_ubo.screen_projection : global_ubo.view_projection) * transform;

    v_uv = a_position;
    v_color = i_color;
    v_size = i_size;
    v_border_color = i_border_color;
    v_border_thickness = i_border_thickness;
    v_border_radius = i_border_radius;
    v_shape = i_shape;
    
    gl_Position = mvp * vec4(a_position, 0, 1);
    gl_Position.z = i_position.z;
})";

static const char GL_SPRITE_FRAG[1227] = R"(#version 330 core

in vec2 v_uv;
flat in vec4 v_color;
flat in vec4 v_outline_color;
flat in float v_outline_thickness;

out vec4 frag_color;

uniform sampler2D u_texture;

void main() {
    vec4 color = v_color;

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

    if (color.a < 0.5) discard;

    frag_color = color;
})";

static const char GL_SPRITE_VERT[2707] = R"(#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_position;
layout(location = 2) in vec4 i_rotation;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_offset;
layout(location = 5) in vec4 i_uv_offset_scale;
layout(location = 6) in vec4 i_color;
layout(location = 7) in vec4 i_outline_color;
layout(location = 8) in float i_outline_thickness;
layout(location = 9) in uint i_flags;

layout(std140) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 nozoom_view_projection;
    mat4 nozoom_projection;
    mat4 transform_matrix;
    mat4 inv_view_proj;
    vec2 camera_position;
    vec2 window_size;
} global_ubo;

out vec2 v_uv;
flat out vec4 v_color;
flat out vec4 v_outline_color;
flat out float v_outline_thickness;

const uint IS_UI_FLAG = 1u << 0u;
const uint FLAG_IGNORE_CAMERA_ZOOM = 1u << 1u;

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
    bool ignore_camera_zoom = (i_flags & FLAG_IGNORE_CAMERA_ZOOM) == FLAG_IGNORE_CAMERA_ZOOM;

    mat4 mvp = (is_ui ? global_ubo.screen_projection : ignore_camera_zoom ? global_ubo.nozoom_view_projection : global_ubo.view_projection) * transform;

    v_uv = a_position * i_uv_offset_scale.zw + i_uv_offset_scale.xy;
    v_color = i_color;
    v_outline_color = i_outline_color;
    v_outline_thickness = i_outline_thickness;
    
    gl_Position = mvp * vec4(a_position, 0, 1);
    gl_Position.z = i_position.z;
})";

static const unsigned char VULKAN_FONT_FRAG[1916] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 81, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 8, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 21, 0, 0, 0, 55, 0, 0, 0, 78, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 100, 105, 115, 116, 0, 0, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 15, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 21, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 28, 0, 0, 0, 119, 105, 100, 116, 104, 0, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 3, 0, 40, 0, 0, 0, 109, 117, 0, 0, 5, 0, 3, 0, 51, 0, 0, 0, 114, 103, 98, 0, 5, 0, 4, 0, 55, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 61, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 5, 0, 78, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 21, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 55, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 55, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 78, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 25, 0, 9, 0, 9, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 13, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 17, 0, 0, 0, 9, 0, 0, 0, 23, 0, 4, 0, 19, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 20, 0, 0, 0, 1, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 20, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 23, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 25, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 25, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 154, 153, 153, 62, 43, 0, 4, 0, 6, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 63, 23, 0, 4, 0, 49, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 50, 0, 0, 0, 7, 0, 0, 0, 49, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 44, 0, 6, 0, 49, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 52, 0, 0, 0, 32, 0, 4, 0, 54, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 59, 0, 4, 0, 54, 0, 0, 0, 55, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 60, 0, 0, 0, 7, 0, 0, 0, 23, 0, 0, 0, 43, 0, 4, 0, 25, 0, 0, 0, 68, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 71, 0, 0, 0, 205, 204, 76, 61, 20, 0, 2, 0, 72, 0, 0, 0, 32, 0, 4, 0, 77, 0, 0, 0, 3, 0, 0, 0, 23, 0, 0, 0, 59, 0, 4, 0, 77, 0, 0, 0, 78, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 80, 0, 0, 0, 205, 204, 76, 62, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 28, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 31, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 50, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 60, 0, 0, 0, 61, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 86, 0, 5, 0, 17, 0, 0, 0, 18, 0, 0, 0, 12, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 22, 0, 0, 0, 21, 0, 0, 0, 87, 0, 5, 0, 23, 0, 0, 0, 24, 0, 0, 0, 18, 0, 0, 0, 22, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 27, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 27, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 29, 0, 0, 0, 8, 0, 0, 0, 209, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 62, 0, 3, 0, 28, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 33, 0, 0, 0, 28, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 32, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 0, 0, 0, 28, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 32, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 37, 0, 0, 0, 8, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 38, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 37, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 39, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 31, 0, 0, 0, 39, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 28, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 43, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 44, 0, 0, 0, 28, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 41, 0, 0, 0, 44, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 46, 0, 0, 0, 8, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 46, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 43, 0, 0, 0, 45, 0, 0, 0, 47, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 56, 0, 0, 0, 55, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 40, 0, 0, 0, 80, 0, 6, 0, 49, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 57, 0, 0, 0, 57, 0, 0, 0, 12, 0, 8, 0, 49, 0, 0, 0, 59, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 53, 0, 0, 0, 56, 0, 0, 0, 58, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 59, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 62, 0, 0, 0, 51, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 0, 0, 0, 31, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 62, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 65, 0, 0, 0, 62, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 66, 0, 0, 0, 62, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 67, 0, 0, 0, 64, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 61, 0, 0, 0, 67, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 69, 0, 0, 0, 61, 0, 0, 0, 68, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 70, 0, 0, 0, 69, 0, 0, 0, 188, 0, 5, 0, 72, 0, 0, 0, 73, 0, 0, 0, 70, 0, 0, 0, 71, 0, 0, 0, 247, 0, 3, 0, 75, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 73, 0, 0, 0, 74, 0, 0, 0, 75, 0, 0, 0, 248, 0, 2, 0, 74, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 75, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 79, 0, 0, 0, 61, 0, 0, 0, 62, 0, 3, 0, 78, 0, 0, 0, 79, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_FONT_VERT[3032] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 85, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 15, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 43, 0, 0, 0, 47, 0, 0, 0, 49, 0, 0, 0, 54, 0, 0, 0, 57, 0, 0, 0, 62, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 71, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 20, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 26, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 26, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 26, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 26, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 26, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 26, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 28, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 5, 0, 40, 0, 0, 0, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 5, 0, 43, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 47, 0, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 49, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 3, 0, 53, 0, 0, 0, 117, 118, 0, 0, 5, 0, 4, 0, 54, 0, 0, 0, 105, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 57, 0, 0, 0, 105, 95, 116, 101, 120, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 62, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 65, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 66, 0, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 69, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 69, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 69, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 69, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 69, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 71, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 26, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 43, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 47, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 49, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 54, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 57, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 62, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 65, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 65, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 66, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 69, 0, 0, 0, 2, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 6, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 21, 0, 4, 0, 9, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 22, 0, 3, 0, 16, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 17, 0, 0, 0, 16, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 18, 0, 0, 0, 17, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 19, 0, 0, 0, 7, 0, 0, 0, 18, 0, 0, 0, 23, 0, 4, 0, 25, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 30, 0, 10, 0, 26, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 25, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 21, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 31, 0, 0, 0, 2, 0, 0, 0, 18, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 39, 0, 0, 0, 7, 0, 0, 0, 25, 0, 0, 0, 23, 0, 4, 0, 41, 0, 0, 0, 16, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 43, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 46, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 49, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 54, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 57, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 61, 0, 0, 0, 3, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 61, 0, 0, 0, 62, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 64, 0, 0, 0, 3, 0, 0, 0, 41, 0, 0, 0, 59, 0, 4, 0, 64, 0, 0, 0, 65, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 66, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 68, 0, 0, 0, 16, 0, 0, 0, 13, 0, 0, 0, 30, 0, 6, 0, 69, 0, 0, 0, 17, 0, 0, 0, 16, 0, 0, 0, 68, 0, 0, 0, 68, 0, 0, 0, 32, 0, 4, 0, 70, 0, 0, 0, 3, 0, 0, 0, 69, 0, 0, 0, 59, 0, 4, 0, 70, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 75, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 80, 0, 0, 0, 3, 0, 0, 0, 17, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 83, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 53, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 199, 0, 5, 0, 9, 0, 0, 0, 14, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 170, 0, 5, 0, 6, 0, 0, 0, 15, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 21, 0, 0, 0, 8, 0, 0, 0, 247, 0, 3, 0, 24, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 21, 0, 0, 0, 23, 0, 0, 0, 34, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 28, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 33, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 33, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 34, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 36, 0, 0, 0, 28, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 37, 0, 0, 0, 36, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 37, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 24, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 38, 0, 0, 0, 22, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 38, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 45, 0, 0, 0, 44, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 52, 0, 0, 0, 45, 0, 0, 0, 51, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 52, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 56, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 59, 0, 0, 0, 56, 0, 0, 0, 58, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 60, 0, 0, 0, 55, 0, 0, 0, 59, 0, 0, 0, 62, 0, 3, 0, 53, 0, 0, 0, 60, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 63, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 62, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 72, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 73, 0, 0, 0, 40, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 76, 0, 0, 0, 73, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 77, 0, 0, 0, 73, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 17, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 74, 0, 0, 0, 75, 0, 0, 0, 145, 0, 5, 0, 17, 0, 0, 0, 79, 0, 0, 0, 72, 0, 0, 0, 78, 0, 0, 0, 65, 0, 5, 0, 80, 0, 0, 0, 81, 0, 0, 0, 71, 0, 0, 0, 30, 0, 0, 0, 62, 0, 3, 0, 81, 0, 0, 0, 79, 0, 0, 0, 65, 0, 6, 0, 83, 0, 0, 0, 84, 0, 0, 0, 71, 0, 0, 0, 30, 0, 0, 0, 82, 0, 0, 0, 62, 0, 3, 0, 84, 0, 0, 0, 75, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_NINEPATCH_FRAG[4536] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 180, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 11, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 106, 0, 0, 0, 117, 0, 0, 0, 119, 0, 0, 0, 124, 0, 0, 0, 166, 0, 0, 0, 178, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 7, 0, 14, 0, 0, 0, 109, 97, 112, 40, 102, 49, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 0, 5, 0, 4, 0, 9, 0, 0, 0, 118, 97, 108, 117, 101, 0, 0, 0, 5, 0, 4, 0, 10, 0, 0, 0, 105, 110, 95, 109, 105, 110, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 105, 110, 95, 109, 97, 120, 0, 0, 5, 0, 4, 0, 12, 0, 0, 0, 111, 117, 116, 95, 109, 105, 110, 0, 5, 0, 4, 0, 13, 0, 0, 0, 111, 117, 116, 95, 109, 97, 120, 0, 5, 0, 9, 0, 22, 0, 0, 0, 112, 114, 111, 99, 101, 115, 115, 95, 97, 120, 105, 115, 40, 102, 49, 59, 118, 102, 50, 59, 118, 102, 50, 59, 0, 0, 0, 0, 5, 0, 4, 0, 19, 0, 0, 0, 99, 111, 111, 114, 100, 0, 0, 0, 5, 0, 6, 0, 20, 0, 0, 0, 115, 111, 117, 114, 99, 101, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 5, 0, 5, 0, 21, 0, 0, 0, 111, 117, 116, 95, 109, 97, 114, 103, 105, 110, 0, 0, 5, 0, 4, 0, 49, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 51, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 52, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 55, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 56, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 76, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 78, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 81, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 82, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 85, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 94, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 96, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 97, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 98, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 99, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 7, 0, 103, 0, 0, 0, 104, 111, 114, 105, 122, 111, 110, 116, 97, 108, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 5, 0, 5, 0, 106, 0, 0, 0, 118, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 111, 0, 0, 0, 118, 101, 114, 116, 105, 99, 97, 108, 95, 109, 97, 114, 103, 105, 110, 0, 5, 0, 4, 0, 115, 0, 0, 0, 110, 101, 119, 95, 117, 118, 0, 0, 5, 0, 4, 0, 117, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 6, 0, 119, 0, 0, 0, 118, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 124, 0, 0, 0, 118, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 4, 0, 128, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 132, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 133, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 143, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 146, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 147, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 152, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 5, 0, 155, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 159, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 166, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 178, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 3, 0, 106, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 106, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 117, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 119, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 119, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 124, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 124, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 155, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 155, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 159, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 159, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 166, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 166, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 178, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 33, 0, 8, 0, 8, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 23, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 33, 0, 6, 0, 18, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 21, 0, 4, 0, 40, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 20, 0, 2, 0, 44, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 62, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 40, 0, 0, 0, 63, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 104, 0, 0, 0, 40, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 105, 0, 0, 0, 1, 0, 0, 0, 104, 0, 0, 0, 59, 0, 4, 0, 105, 0, 0, 0, 106, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 107, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 116, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 117, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 129, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 150, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 151, 0, 0, 0, 7, 0, 0, 0, 150, 0, 0, 0, 25, 0, 9, 0, 153, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 154, 0, 0, 0, 0, 0, 0, 0, 153, 0, 0, 0, 59, 0, 4, 0, 154, 0, 0, 0, 155, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 157, 0, 0, 0, 32, 0, 4, 0, 158, 0, 0, 0, 0, 0, 0, 0, 157, 0, 0, 0, 59, 0, 4, 0, 158, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 161, 0, 0, 0, 153, 0, 0, 0, 32, 0, 4, 0, 165, 0, 0, 0, 1, 0, 0, 0, 150, 0, 0, 0, 59, 0, 4, 0, 165, 0, 0, 0, 166, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 169, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 172, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 177, 0, 0, 0, 3, 0, 0, 0, 150, 0, 0, 0, 59, 0, 4, 0, 177, 0, 0, 0, 178, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 103, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 111, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 115, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 128, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 132, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 133, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 143, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 146, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 147, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 151, 0, 0, 0, 152, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 104, 0, 0, 0, 108, 0, 0, 0, 106, 0, 0, 0, 79, 0, 7, 0, 107, 0, 0, 0, 109, 0, 0, 0, 108, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 112, 0, 4, 0, 16, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 62, 0, 3, 0, 103, 0, 0, 0, 110, 0, 0, 0, 61, 0, 4, 0, 104, 0, 0, 0, 112, 0, 0, 0, 106, 0, 0, 0, 79, 0, 7, 0, 107, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 112, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 112, 0, 4, 0, 16, 0, 0, 0, 114, 0, 0, 0, 113, 0, 0, 0, 62, 0, 3, 0, 111, 0, 0, 0, 114, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 118, 0, 0, 0, 103, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 120, 0, 0, 0, 119, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 122, 0, 0, 0, 118, 0, 0, 0, 121, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 123, 0, 0, 0, 103, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 125, 0, 0, 0, 124, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 126, 0, 0, 0, 125, 0, 0, 0, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 127, 0, 0, 0, 123, 0, 0, 0, 126, 0, 0, 0, 65, 0, 5, 0, 129, 0, 0, 0, 130, 0, 0, 0, 117, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 131, 0, 0, 0, 130, 0, 0, 0, 62, 0, 3, 0, 128, 0, 0, 0, 131, 0, 0, 0, 62, 0, 3, 0, 132, 0, 0, 0, 122, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 127, 0, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 134, 0, 0, 0, 22, 0, 0, 0, 128, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 135, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 136, 0, 0, 0, 119, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 137, 0, 0, 0, 136, 0, 0, 0, 136, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 138, 0, 0, 0, 135, 0, 0, 0, 137, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 139, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 140, 0, 0, 0, 124, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 141, 0, 0, 0, 140, 0, 0, 0, 140, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 142, 0, 0, 0, 139, 0, 0, 0, 141, 0, 0, 0, 65, 0, 5, 0, 129, 0, 0, 0, 144, 0, 0, 0, 117, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 145, 0, 0, 0, 144, 0, 0, 0, 62, 0, 3, 0, 143, 0, 0, 0, 145, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 138, 0, 0, 0, 62, 0, 3, 0, 147, 0, 0, 0, 142, 0, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 148, 0, 0, 0, 22, 0, 0, 0, 143, 0, 0, 0, 146, 0, 0, 0, 147, 0, 0, 0, 80, 0, 5, 0, 16, 0, 0, 0, 149, 0, 0, 0, 134, 0, 0, 0, 148, 0, 0, 0, 62, 0, 3, 0, 115, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 153, 0, 0, 0, 156, 0, 0, 0, 155, 0, 0, 0, 61, 0, 4, 0, 157, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 86, 0, 5, 0, 161, 0, 0, 0, 162, 0, 0, 0, 156, 0, 0, 0, 160, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 163, 0, 0, 0, 115, 0, 0, 0, 87, 0, 5, 0, 150, 0, 0, 0, 164, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 61, 0, 4, 0, 150, 0, 0, 0, 167, 0, 0, 0, 166, 0, 0, 0, 133, 0, 5, 0, 150, 0, 0, 0, 168, 0, 0, 0, 164, 0, 0, 0, 167, 0, 0, 0, 62, 0, 3, 0, 152, 0, 0, 0, 168, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 170, 0, 0, 0, 152, 0, 0, 0, 169, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 171, 0, 0, 0, 170, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 173, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 247, 0, 3, 0, 175, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 173, 0, 0, 0, 174, 0, 0, 0, 175, 0, 0, 0, 248, 0, 2, 0, 174, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 175, 0, 0, 0, 61, 0, 4, 0, 150, 0, 0, 0, 179, 0, 0, 0, 152, 0, 0, 0, 62, 0, 3, 0, 178, 0, 0, 0, 179, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 9, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 10, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 11, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 12, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 13, 0, 0, 0, 248, 0, 2, 0, 15, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 24, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 10, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 24, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 27, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 28, 0, 0, 0, 10, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 29, 0, 0, 0, 27, 0, 0, 0, 28, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 30, 0, 0, 0, 26, 0, 0, 0, 29, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 31, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 12, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 31, 0, 0, 0, 32, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 30, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 0, 0, 0, 12, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 34, 0, 0, 0, 35, 0, 0, 0, 254, 0, 2, 0, 36, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 19, 0, 0, 0, 55, 0, 3, 0, 17, 0, 0, 0, 20, 0, 0, 0, 55, 0, 3, 0, 17, 0, 0, 0, 21, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 49, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 55, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 56, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 76, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 78, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 81, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 82, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 85, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 94, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 96, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 97, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 98, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 99, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 39, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 42, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 43, 0, 0, 0, 42, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 45, 0, 0, 0, 39, 0, 0, 0, 43, 0, 0, 0, 247, 0, 3, 0, 47, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 45, 0, 0, 0, 46, 0, 0, 0, 47, 0, 0, 0, 248, 0, 2, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 49, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 48, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 53, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 54, 0, 0, 0, 62, 0, 3, 0, 55, 0, 0, 0, 48, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 57, 0, 0, 0, 20, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 56, 0, 0, 0, 58, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 59, 0, 0, 0, 14, 0, 0, 0, 49, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 55, 0, 0, 0, 56, 0, 0, 0, 254, 0, 2, 0, 59, 0, 0, 0, 248, 0, 2, 0, 47, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 64, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 66, 0, 0, 0, 62, 0, 0, 0, 65, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 67, 0, 0, 0, 61, 0, 0, 0, 66, 0, 0, 0, 247, 0, 3, 0, 69, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 67, 0, 0, 0, 68, 0, 0, 0, 69, 0, 0, 0, 248, 0, 2, 0, 68, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 70, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 72, 0, 0, 0, 62, 0, 0, 0, 71, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 73, 0, 0, 0, 20, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 73, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 75, 0, 0, 0, 62, 0, 0, 0, 74, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 76, 0, 0, 0, 77, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 79, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 80, 0, 0, 0, 79, 0, 0, 0, 62, 0, 3, 0, 78, 0, 0, 0, 80, 0, 0, 0, 62, 0, 3, 0, 81, 0, 0, 0, 72, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 83, 0, 0, 0, 20, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 84, 0, 0, 0, 83, 0, 0, 0, 62, 0, 3, 0, 82, 0, 0, 0, 84, 0, 0, 0, 62, 0, 3, 0, 85, 0, 0, 0, 75, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 86, 0, 0, 0, 14, 0, 0, 0, 76, 0, 0, 0, 78, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 85, 0, 0, 0, 254, 0, 2, 0, 86, 0, 0, 0, 248, 0, 2, 0, 69, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 88, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 88, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 90, 0, 0, 0, 62, 0, 0, 0, 89, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 91, 0, 0, 0, 20, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 62, 0, 0, 0, 92, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 94, 0, 0, 0, 95, 0, 0, 0, 62, 0, 3, 0, 96, 0, 0, 0, 90, 0, 0, 0, 62, 0, 3, 0, 97, 0, 0, 0, 62, 0, 0, 0, 62, 0, 3, 0, 98, 0, 0, 0, 93, 0, 0, 0, 62, 0, 3, 0, 99, 0, 0, 0, 62, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 100, 0, 0, 0, 14, 0, 0, 0, 94, 0, 0, 0, 96, 0, 0, 0, 97, 0, 0, 0, 98, 0, 0, 0, 99, 0, 0, 0, 254, 0, 2, 0, 100, 0, 0, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_NINEPATCH_VERT[7908] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 45, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 22, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 147, 0, 0, 0, 181, 0, 0, 0, 184, 0, 0, 0, 228, 0, 0, 0, 6, 1, 0, 0, 7, 1, 0, 0, 9, 1, 0, 0, 17, 1, 0, 0, 18, 1, 0, 0, 22, 1, 0, 0, 24, 1, 0, 0, 26, 1, 0, 0, 27, 1, 0, 0, 29, 1, 0, 0, 30, 1, 0, 0, 35, 1, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 3, 0, 8, 0, 0, 0, 113, 120, 120, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 27, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 34, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 40, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 46, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 52, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 59, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 65, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 73, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 5, 0, 141, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 147, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 179, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 181, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 184, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 226, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 228, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 7, 0, 232, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 3, 0, 236, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 241, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 241, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 9, 0, 241, 0, 0, 0, 2, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 3, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 241, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 243, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 6, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 7, 1, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 7, 0, 9, 1, 0, 0, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 4, 0, 17, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 18, 1, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 22, 1, 0, 0, 118, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 24, 1, 0, 0, 105, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 26, 1, 0, 0, 118, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 27, 1, 0, 0, 105, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 29, 1, 0, 0, 118, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 30, 1, 0, 0, 105, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 33, 1, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 33, 1, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 33, 1, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 33, 1, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 33, 1, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 35, 1, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 147, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 181, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 184, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 228, 0, 0, 0, 30, 0, 0, 0, 10, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 241, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 6, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 9, 1, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 3, 0, 17, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 17, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 18, 1, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 3, 0, 22, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 22, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 24, 1, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 26, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 26, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 27, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 29, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 29, 1, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 30, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 33, 1, 0, 0, 2, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 12, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 53, 0, 0, 0, 3, 0, 0, 0, 24, 0, 4, 0, 71, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 72, 0, 0, 0, 7, 0, 0, 0, 71, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 119, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 142, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 143, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 144, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 23, 0, 4, 0, 145, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 146, 0, 0, 0, 1, 0, 0, 0, 145, 0, 0, 0, 59, 0, 4, 0, 146, 0, 0, 0, 147, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 177, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 178, 0, 0, 0, 7, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 180, 0, 0, 0, 1, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 181, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 187, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 188, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 190, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 196, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 203, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 224, 0, 0, 0, 32, 0, 4, 0, 225, 0, 0, 0, 7, 0, 0, 0, 224, 0, 0, 0, 32, 0, 4, 0, 227, 0, 0, 0, 1, 0, 0, 0, 12, 0, 0, 0, 59, 0, 4, 0, 227, 0, 0, 0, 228, 0, 0, 0, 1, 0, 0, 0, 30, 0, 10, 0, 241, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 177, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 242, 0, 0, 0, 2, 0, 0, 0, 241, 0, 0, 0, 59, 0, 4, 0, 242, 0, 0, 0, 243, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 244, 0, 0, 0, 2, 0, 0, 0, 71, 0, 0, 0, 32, 0, 4, 0, 5, 1, 0, 0, 3, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 6, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 7, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 9, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 16, 1, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 17, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 18, 1, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 20, 1, 0, 0, 12, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 21, 1, 0, 0, 3, 0, 0, 0, 20, 1, 0, 0, 59, 0, 4, 0, 21, 1, 0, 0, 22, 1, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 23, 1, 0, 0, 1, 0, 0, 0, 20, 1, 0, 0, 59, 0, 4, 0, 23, 1, 0, 0, 24, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 26, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 27, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 29, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 30, 1, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 32, 1, 0, 0, 6, 0, 0, 0, 21, 0, 0, 0, 30, 0, 6, 0, 33, 1, 0, 0, 9, 0, 0, 0, 6, 0, 0, 0, 32, 1, 0, 0, 32, 1, 0, 0, 32, 0, 4, 0, 34, 1, 0, 0, 3, 0, 0, 0, 33, 1, 0, 0, 59, 0, 4, 0, 34, 1, 0, 0, 35, 1, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 43, 1, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 27, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 34, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 59, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 73, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 141, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 178, 0, 0, 0, 179, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 226, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 232, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 236, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 238, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 249, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 15, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 17, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 18, 0, 0, 0, 17, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 19, 0, 0, 0, 16, 0, 0, 0, 18, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 22, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 24, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 25, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 29, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 31, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 31, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 30, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 27, 0, 0, 0, 33, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 35, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 37, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 34, 0, 0, 0, 39, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 41, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 43, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 42, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 45, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 47, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 49, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 51, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 54, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 56, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 60, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 62, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 59, 0, 0, 0, 64, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 66, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 68, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 70, 0, 0, 0, 67, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 79, 0, 0, 0, 75, 0, 0, 0, 78, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 80, 0, 0, 0, 74, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 81, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 65, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 84, 0, 0, 0, 75, 0, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 59, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 87, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 88, 0, 0, 0, 75, 0, 0, 0, 87, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 90, 0, 0, 0, 80, 0, 0, 0, 84, 0, 0, 0, 88, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 65, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 91, 0, 0, 0, 92, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 75, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 97, 0, 0, 0, 95, 0, 0, 0, 96, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 98, 0, 0, 0, 75, 0, 0, 0, 97, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 99, 0, 0, 0, 74, 0, 0, 0, 98, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 100, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 52, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 103, 0, 0, 0, 75, 0, 0, 0, 102, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 104, 0, 0, 0, 94, 0, 0, 0, 99, 0, 0, 0, 103, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 105, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 59, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 108, 0, 0, 0, 75, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 111, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 112, 0, 0, 0, 75, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 113, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 20, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 114, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 116, 0, 0, 0, 75, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 117, 0, 0, 0, 74, 0, 0, 0, 116, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 118, 0, 0, 0, 108, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 89, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 120, 0, 0, 0, 90, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 121, 0, 0, 0, 90, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 122, 0, 0, 0, 90, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 123, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 124, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 125, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 126, 0, 0, 0, 104, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 104, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 128, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 130, 0, 0, 0, 118, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 131, 0, 0, 0, 118, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 132, 0, 0, 0, 119, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 133, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 134, 0, 0, 0, 119, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 135, 0, 0, 0, 119, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 136, 0, 0, 0, 120, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 123, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 137, 0, 0, 0, 124, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 138, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 139, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 140, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 139, 0, 0, 0, 62, 0, 3, 0, 73, 0, 0, 0, 140, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 150, 0, 0, 0, 147, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 152, 0, 0, 0, 149, 0, 0, 0, 151, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 153, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 154, 0, 0, 0, 142, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 155, 0, 0, 0, 142, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 156, 0, 0, 0, 142, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 157, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 158, 0, 0, 0, 143, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 159, 0, 0, 0, 143, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 143, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 161, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 162, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 163, 0, 0, 0, 144, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 164, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 165, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 166, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 167, 0, 0, 0, 152, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 168, 0, 0, 0, 152, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 169, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 156, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 170, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 159, 0, 0, 0, 160, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 171, 0, 0, 0, 161, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 164, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 172, 0, 0, 0, 165, 0, 0, 0, 166, 0, 0, 0, 167, 0, 0, 0, 168, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 173, 0, 0, 0, 169, 0, 0, 0, 170, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 173, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 174, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 175, 0, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 176, 0, 0, 0, 175, 0, 0, 0, 174, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 176, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 182, 0, 0, 0, 181, 0, 0, 0, 127, 0, 4, 0, 177, 0, 0, 0, 183, 0, 0, 0, 182, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 185, 0, 0, 0, 184, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 186, 0, 0, 0, 183, 0, 0, 0, 185, 0, 0, 0, 62, 0, 3, 0, 179, 0, 0, 0, 186, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 191, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 192, 0, 0, 0, 191, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 193, 0, 0, 0, 179, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 194, 0, 0, 0, 193, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 195, 0, 0, 0, 192, 0, 0, 0, 194, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 197, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 198, 0, 0, 0, 197, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 199, 0, 0, 0, 179, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 200, 0, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 201, 0, 0, 0, 198, 0, 0, 0, 200, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 202, 0, 0, 0, 195, 0, 0, 0, 201, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 204, 0, 0, 0, 141, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 89, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 207, 0, 0, 0, 202, 0, 0, 0, 206, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 208, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 209, 0, 0, 0, 208, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 210, 0, 0, 0, 207, 0, 0, 0, 209, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 211, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 62, 0, 3, 0, 211, 0, 0, 0, 210, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 212, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 213, 0, 0, 0, 212, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 214, 0, 0, 0, 184, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 215, 0, 0, 0, 214, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 216, 0, 0, 0, 213, 0, 0, 0, 215, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 217, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 217, 0, 0, 0, 216, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 218, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 219, 0, 0, 0, 218, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 220, 0, 0, 0, 184, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 221, 0, 0, 0, 220, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 222, 0, 0, 0, 219, 0, 0, 0, 221, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 223, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 62, 0, 3, 0, 223, 0, 0, 0, 222, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 229, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 230, 0, 0, 0, 229, 0, 0, 0, 21, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 231, 0, 0, 0, 230, 0, 0, 0, 21, 0, 0, 0, 62, 0, 3, 0, 226, 0, 0, 0, 231, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 233, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 28, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 232, 0, 0, 0, 235, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 237, 0, 0, 0, 226, 0, 0, 0, 247, 0, 3, 0, 240, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 237, 0, 0, 0, 239, 0, 0, 0, 247, 0, 0, 0, 248, 0, 2, 0, 239, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 245, 0, 0, 0, 243, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 246, 0, 0, 0, 245, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 246, 0, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 247, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 248, 0, 0, 0, 232, 0, 0, 0, 247, 0, 3, 0, 251, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 248, 0, 0, 0, 250, 0, 0, 0, 254, 0, 0, 0, 248, 0, 2, 0, 250, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 252, 0, 0, 0, 243, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 253, 0, 0, 0, 252, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 253, 0, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 254, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 255, 0, 0, 0, 243, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 0, 1, 0, 0, 255, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 0, 1, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 251, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 1, 1, 0, 0, 249, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 1, 1, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 240, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 2, 1, 0, 0, 238, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 3, 1, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 4, 1, 0, 0, 2, 1, 0, 0, 3, 1, 0, 0, 62, 0, 3, 0, 236, 0, 0, 0, 4, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 10, 1, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 12, 1, 0, 0, 8, 1, 0, 0, 11, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 13, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 14, 1, 0, 0, 13, 1, 0, 0, 13, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 177, 0, 0, 0, 15, 1, 0, 0, 12, 1, 0, 0, 14, 1, 0, 0, 62, 0, 3, 0, 6, 1, 0, 0, 15, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 19, 1, 0, 0, 18, 1, 0, 0, 62, 0, 3, 0, 17, 1, 0, 0, 19, 1, 0, 0, 61, 0, 4, 0, 20, 1, 0, 0, 25, 1, 0, 0, 24, 1, 0, 0, 62, 0, 3, 0, 22, 1, 0, 0, 25, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 28, 1, 0, 0, 27, 1, 0, 0, 62, 0, 3, 0, 26, 1, 0, 0, 28, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 31, 1, 0, 0, 30, 1, 0, 0, 62, 0, 3, 0, 29, 1, 0, 0, 31, 1, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 36, 1, 0, 0, 236, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 37, 1, 0, 0, 7, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 38, 1, 0, 0, 37, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 39, 1, 0, 0, 37, 1, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 40, 1, 0, 0, 38, 1, 0, 0, 39, 1, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 145, 0, 5, 0, 9, 0, 0, 0, 41, 1, 0, 0, 36, 1, 0, 0, 40, 1, 0, 0, 65, 0, 5, 0, 16, 1, 0, 0, 42, 1, 0, 0, 35, 1, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 42, 1, 0, 0, 41, 1, 0, 0, 65, 0, 6, 0, 43, 1, 0, 0, 44, 1, 0, 0, 35, 1, 0, 0, 189, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 44, 1, 0, 0, 74, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SHAPE_FRAG[10632] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 202, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 13, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 15, 1, 0, 0, 17, 1, 0, 0, 20, 1, 0, 0, 26, 1, 0, 0, 27, 1, 0, 0, 29, 1, 0, 0, 46, 1, 0, 0, 54, 1, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 8, 0, 17, 0, 0, 0, 99, 105, 114, 99, 108, 101, 40, 118, 102, 50, 59, 118, 102, 52, 59, 118, 102, 52, 59, 102, 49, 59, 0, 0, 5, 0, 3, 0, 13, 0, 0, 0, 115, 116, 0, 0, 5, 0, 4, 0, 14, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 6, 0, 15, 0, 0, 0, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 7, 0, 16, 0, 0, 0, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 0, 5, 0, 5, 0, 22, 0, 0, 0, 109, 111, 100, 98, 40, 102, 49, 59, 102, 49, 59, 0, 5, 0, 3, 0, 20, 0, 0, 0, 120, 0, 0, 0, 5, 0, 3, 0, 21, 0, 0, 0, 121, 0, 0, 0, 5, 0, 8, 0, 29, 0, 0, 0, 97, 114, 99, 95, 115, 100, 102, 40, 118, 102, 50, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 0, 0, 0, 5, 0, 3, 0, 25, 0, 0, 0, 112, 0, 0, 0, 5, 0, 3, 0, 26, 0, 0, 0, 97, 48, 0, 0, 5, 0, 3, 0, 27, 0, 0, 0, 97, 49, 0, 0, 5, 0, 3, 0, 28, 0, 0, 0, 114, 0, 0, 0, 5, 0, 10, 0, 35, 0, 0, 0, 114, 111, 117, 110, 100, 101, 100, 95, 98, 111, 120, 95, 115, 100, 102, 40, 118, 102, 50, 59, 118, 102, 50, 59, 118, 102, 52, 59, 0, 0, 0, 0, 5, 0, 3, 0, 32, 0, 0, 0, 117, 118, 0, 0, 5, 0, 4, 0, 33, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 4, 0, 34, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 4, 0, 37, 0, 0, 0, 100, 105, 115, 116, 0, 0, 0, 0, 5, 0, 3, 0, 42, 0, 0, 0, 100, 0, 0, 0, 5, 0, 5, 0, 46, 0, 0, 0, 98, 111, 114, 100, 101, 114, 95, 99, 114, 0, 0, 0, 5, 0, 6, 0, 50, 0, 0, 0, 98, 111, 114, 100, 101, 114, 95, 119, 101, 105, 103, 104, 116, 0, 0, 0, 5, 0, 3, 0, 65, 0, 0, 0, 99, 114, 0, 0, 5, 0, 4, 0, 66, 0, 0, 0, 119, 101, 105, 103, 104, 116, 0, 0, 5, 0, 3, 0, 80, 0, 0, 0, 116, 49, 0, 0, 5, 0, 3, 0, 98, 0, 0, 0, 116, 50, 0, 0, 5, 0, 3, 0, 136, 0, 0, 0, 97, 0, 0, 0, 5, 0, 4, 0, 143, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 144, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 3, 0, 146, 0, 0, 0, 97, 112, 0, 0, 5, 0, 3, 0, 157, 0, 0, 0, 97, 49, 112, 0, 5, 0, 3, 0, 172, 0, 0, 0, 113, 48, 0, 0, 5, 0, 3, 0, 182, 0, 0, 0, 113, 49, 0, 0, 5, 0, 4, 0, 209, 0, 0, 0, 99, 101, 110, 116, 101, 114, 0, 0, 5, 0, 4, 0, 244, 0, 0, 0, 100, 105, 115, 116, 0, 0, 0, 0, 5, 0, 5, 0, 15, 1, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 4, 0, 17, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 20, 1, 0, 0, 118, 95, 115, 104, 97, 112, 101, 0, 5, 0, 4, 0, 26, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 6, 0, 27, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 7, 0, 29, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 4, 0, 30, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 32, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 34, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 36, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 5, 0, 45, 1, 0, 0, 115, 116, 97, 114, 116, 95, 97, 110, 103, 108, 101, 0, 5, 0, 6, 0, 46, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 5, 0, 49, 1, 0, 0, 101, 110, 100, 95, 97, 110, 103, 108, 101, 0, 0, 0, 5, 0, 5, 0, 52, 1, 0, 0, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 5, 0, 4, 0, 54, 1, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 3, 0, 59, 1, 0, 0, 112, 0, 0, 0, 5, 0, 3, 0, 76, 1, 0, 0, 100, 0, 0, 0, 5, 0, 4, 0, 79, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 81, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 83, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 85, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 3, 0, 87, 1, 0, 0, 97, 97, 0, 0, 5, 0, 4, 0, 94, 1, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 4, 0, 114, 1, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 3, 0, 130, 1, 0, 0, 100, 0, 0, 0, 5, 0, 4, 0, 131, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 133, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 135, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 3, 0, 138, 1, 0, 0, 97, 97, 0, 0, 5, 0, 6, 0, 145, 1, 0, 0, 115, 109, 111, 111, 116, 104, 101, 100, 95, 97, 108, 112, 104, 97, 0, 0, 5, 0, 6, 0, 151, 1, 0, 0, 98, 111, 114, 100, 101, 114, 95, 97, 108, 112, 104, 97, 0, 0, 0, 0, 5, 0, 5, 0, 160, 1, 0, 0, 113, 117, 97, 100, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 8, 0, 171, 1, 0, 0, 113, 117, 97, 100, 95, 99, 111, 108, 111, 114, 95, 119, 105, 116, 104, 95, 98, 111, 114, 100, 101, 114, 0, 0, 5, 0, 4, 0, 182, 1, 0, 0, 114, 101, 115, 117, 108, 116, 0, 0, 71, 0, 4, 0, 15, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 17, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 17, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 20, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 20, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 26, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 27, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 27, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 29, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 29, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 46, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 46, 1, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 54, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 54, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 32, 0, 4, 0, 11, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 33, 0, 7, 0, 12, 0, 0, 0, 9, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 33, 0, 5, 0, 19, 0, 0, 0, 6, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 33, 0, 7, 0, 24, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 33, 0, 6, 0, 31, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 39, 0, 0, 0, 0, 0, 0, 63, 44, 0, 5, 0, 7, 0, 0, 0, 40, 0, 0, 0, 39, 0, 0, 0, 39, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 111, 18, 131, 58, 43, 0, 4, 0, 6, 0, 0, 0, 81, 0, 0, 0, 0, 0, 128, 63, 21, 0, 4, 0, 83, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 83, 0, 0, 0, 84, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 83, 0, 0, 0, 88, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 0, 0, 0, 0, 23, 0, 4, 0, 111, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 142, 0, 0, 0, 219, 15, 201, 64, 20, 0, 2, 0, 151, 0, 0, 0, 32, 0, 4, 0, 14, 1, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 14, 1, 0, 0, 15, 1, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 16, 1, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 17, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 19, 1, 0, 0, 1, 0, 0, 0, 83, 0, 0, 0, 59, 0, 4, 0, 19, 1, 0, 0, 20, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 25, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 25, 1, 0, 0, 26, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 27, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 28, 1, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 28, 1, 0, 0, 29, 1, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 83, 0, 0, 0, 41, 1, 0, 0, 2, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 46, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 25, 1, 0, 0, 54, 1, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 66, 1, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 83, 0, 0, 0, 104, 1, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 195, 1, 0, 0, 10, 215, 35, 60, 43, 0, 4, 0, 6, 0, 0, 0, 201, 1, 0, 0, 219, 15, 73, 64, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 30, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 32, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 34, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 36, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 45, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 49, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 52, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 59, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 76, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 79, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 81, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 83, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 85, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 87, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 94, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 114, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 130, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 131, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 133, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 135, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 138, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 145, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 151, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 160, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 171, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 182, 1, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 18, 1, 0, 0, 17, 1, 0, 0, 62, 0, 3, 0, 15, 1, 0, 0, 18, 1, 0, 0, 61, 0, 4, 0, 83, 0, 0, 0, 21, 1, 0, 0, 20, 1, 0, 0, 170, 0, 5, 0, 151, 0, 0, 0, 22, 1, 0, 0, 21, 1, 0, 0, 84, 0, 0, 0, 247, 0, 3, 0, 24, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 22, 1, 0, 0, 23, 1, 0, 0, 39, 1, 0, 0, 248, 0, 2, 0, 23, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 31, 1, 0, 0, 26, 1, 0, 0, 62, 0, 3, 0, 30, 1, 0, 0, 31, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 33, 1, 0, 0, 17, 1, 0, 0, 62, 0, 3, 0, 32, 1, 0, 0, 33, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 35, 1, 0, 0, 27, 1, 0, 0, 62, 0, 3, 0, 34, 1, 0, 0, 35, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 37, 1, 0, 0, 29, 1, 0, 0, 62, 0, 3, 0, 36, 1, 0, 0, 37, 1, 0, 0, 57, 0, 8, 0, 9, 0, 0, 0, 38, 1, 0, 0, 17, 0, 0, 0, 30, 1, 0, 0, 32, 1, 0, 0, 34, 1, 0, 0, 36, 1, 0, 0, 62, 0, 3, 0, 15, 1, 0, 0, 38, 1, 0, 0, 249, 0, 2, 0, 24, 1, 0, 0, 248, 0, 2, 0, 39, 1, 0, 0, 61, 0, 4, 0, 83, 0, 0, 0, 40, 1, 0, 0, 20, 1, 0, 0, 170, 0, 5, 0, 151, 0, 0, 0, 42, 1, 0, 0, 40, 1, 0, 0, 41, 1, 0, 0, 247, 0, 3, 0, 44, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 42, 1, 0, 0, 43, 1, 0, 0, 113, 1, 0, 0, 248, 0, 2, 0, 43, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 47, 1, 0, 0, 46, 1, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 1, 0, 0, 47, 1, 0, 0, 62, 0, 3, 0, 45, 1, 0, 0, 48, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 50, 1, 0, 0, 46, 1, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 51, 1, 0, 0, 50, 1, 0, 0, 62, 0, 3, 0, 49, 1, 0, 0, 51, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 53, 1, 0, 0, 29, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 55, 1, 0, 0, 54, 1, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 56, 1, 0, 0, 55, 1, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 57, 1, 0, 0, 53, 1, 0, 0, 56, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 58, 1, 0, 0, 39, 0, 0, 0, 57, 1, 0, 0, 62, 0, 3, 0, 52, 1, 0, 0, 58, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 60, 1, 0, 0, 26, 1, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 1, 0, 0, 60, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 62, 1, 0, 0, 26, 1, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 1, 0, 0, 62, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 64, 1, 0, 0, 81, 0, 0, 0, 63, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 65, 1, 0, 0, 61, 1, 0, 0, 64, 1, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 67, 1, 0, 0, 65, 1, 0, 0, 66, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 68, 1, 0, 0, 81, 0, 0, 0, 81, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 69, 1, 0, 0, 67, 1, 0, 0, 68, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 70, 1, 0, 0, 54, 1, 0, 0, 133, 0, 5, 0, 7, 0, 0, 0, 71, 1, 0, 0, 69, 1, 0, 0, 70, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 72, 1, 0, 0, 54, 1, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 73, 1, 0, 0, 72, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 74, 1, 0, 0, 73, 1, 0, 0, 73, 1, 0, 0, 136, 0, 5, 0, 7, 0, 0, 0, 75, 1, 0, 0, 71, 1, 0, 0, 74, 1, 0, 0, 62, 0, 3, 0, 59, 1, 0, 0, 75, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 1, 0, 0, 52, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 78, 1, 0, 0, 81, 0, 0, 0, 77, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 80, 1, 0, 0, 59, 1, 0, 0, 62, 0, 3, 0, 79, 1, 0, 0, 80, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 1, 0, 0, 45, 1, 0, 0, 62, 0, 3, 0, 81, 1, 0, 0, 82, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 84, 1, 0, 0, 49, 1, 0, 0, 62, 0, 3, 0, 83, 1, 0, 0, 84, 1, 0, 0, 62, 0, 3, 0, 85, 1, 0, 0, 78, 1, 0, 0, 57, 0, 8, 0, 6, 0, 0, 0, 86, 1, 0, 0, 29, 0, 0, 0, 79, 1, 0, 0, 81, 1, 0, 0, 83, 1, 0, 0, 85, 1, 0, 0, 62, 0, 3, 0, 76, 1, 0, 0, 86, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 88, 1, 0, 0, 76, 1, 0, 0, 207, 0, 4, 0, 6, 0, 0, 0, 89, 1, 0, 0, 88, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 90, 1, 0, 0, 76, 1, 0, 0, 208, 0, 4, 0, 6, 0, 0, 0, 91, 1, 0, 0, 90, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 92, 1, 0, 0, 89, 1, 0, 0, 91, 1, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 93, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 92, 1, 0, 0, 62, 0, 3, 0, 87, 1, 0, 0, 93, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 1, 0, 0, 52, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 1, 0, 0, 87, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 97, 1, 0, 0, 95, 1, 0, 0, 96, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 98, 1, 0, 0, 52, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 99, 1, 0, 0, 76, 1, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 100, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 97, 1, 0, 0, 98, 1, 0, 0, 99, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 101, 1, 0, 0, 81, 0, 0, 0, 100, 1, 0, 0, 62, 0, 3, 0, 94, 1, 0, 0, 101, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 102, 1, 0, 0, 17, 1, 0, 0, 79, 0, 8, 0, 111, 0, 0, 0, 103, 1, 0, 0, 102, 1, 0, 0, 102, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 105, 1, 0, 0, 17, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 1, 0, 0, 105, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 107, 1, 0, 0, 94, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 108, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 106, 1, 0, 0, 107, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 109, 1, 0, 0, 103, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 110, 1, 0, 0, 103, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 111, 1, 0, 0, 103, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 112, 1, 0, 0, 109, 1, 0, 0, 110, 1, 0, 0, 111, 1, 0, 0, 108, 1, 0, 0, 62, 0, 3, 0, 15, 1, 0, 0, 112, 1, 0, 0, 249, 0, 2, 0, 44, 1, 0, 0, 248, 0, 2, 0, 113, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 115, 1, 0, 0, 46, 1, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 116, 1, 0, 0, 115, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 117, 1, 0, 0, 46, 1, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 118, 1, 0, 0, 117, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 119, 1, 0, 0, 46, 1, 0, 0, 41, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 120, 1, 0, 0, 119, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 121, 1, 0, 0, 46, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 122, 1, 0, 0, 121, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 123, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 120, 1, 0, 0, 122, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 124, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 118, 1, 0, 0, 123, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 125, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 116, 1, 0, 0, 124, 1, 0, 0, 62, 0, 3, 0, 114, 1, 0, 0, 125, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 126, 1, 0, 0, 114, 1, 0, 0, 186, 0, 5, 0, 151, 0, 0, 0, 127, 1, 0, 0, 126, 1, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 129, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 127, 1, 0, 0, 128, 1, 0, 0, 129, 1, 0, 0, 248, 0, 2, 0, 128, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 132, 1, 0, 0, 26, 1, 0, 0, 62, 0, 3, 0, 131, 1, 0, 0, 132, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 134, 1, 0, 0, 54, 1, 0, 0, 62, 0, 3, 0, 133, 1, 0, 0, 134, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 136, 1, 0, 0, 46, 1, 0, 0, 62, 0, 3, 0, 135, 1, 0, 0, 136, 1, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 137, 1, 0, 0, 35, 0, 0, 0, 131, 1, 0, 0, 133, 1, 0, 0, 135, 1, 0, 0, 62, 0, 3, 0, 130, 1, 0, 0, 137, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 139, 1, 0, 0, 130, 1, 0, 0, 207, 0, 4, 0, 6, 0, 0, 0, 140, 1, 0, 0, 139, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 141, 1, 0, 0, 130, 1, 0, 0, 208, 0, 4, 0, 6, 0, 0, 0, 142, 1, 0, 0, 141, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 143, 1, 0, 0, 140, 1, 0, 0, 142, 1, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 144, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 143, 1, 0, 0, 62, 0, 3, 0, 138, 1, 0, 0, 144, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 146, 1, 0, 0, 138, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 147, 1, 0, 0, 95, 0, 0, 0, 146, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 148, 1, 0, 0, 130, 1, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 149, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 147, 1, 0, 0, 95, 0, 0, 0, 148, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 150, 1, 0, 0, 81, 0, 0, 0, 149, 1, 0, 0, 62, 0, 3, 0, 145, 1, 0, 0, 150, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 152, 1, 0, 0, 29, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 153, 1, 0, 0, 138, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 154, 1, 0, 0, 152, 1, 0, 0, 153, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 155, 1, 0, 0, 29, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 156, 1, 0, 0, 130, 1, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 157, 1, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 156, 1, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 158, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 154, 1, 0, 0, 155, 1, 0, 0, 157, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 159, 1, 0, 0, 81, 0, 0, 0, 158, 1, 0, 0, 62, 0, 3, 0, 151, 1, 0, 0, 159, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 161, 1, 0, 0, 17, 1, 0, 0, 79, 0, 8, 0, 111, 0, 0, 0, 162, 1, 0, 0, 161, 1, 0, 0, 161, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 163, 1, 0, 0, 17, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 164, 1, 0, 0, 163, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 165, 1, 0, 0, 145, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 166, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 164, 1, 0, 0, 165, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 167, 1, 0, 0, 162, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 168, 1, 0, 0, 162, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 169, 1, 0, 0, 162, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 170, 1, 0, 0, 167, 1, 0, 0, 168, 1, 0, 0, 169, 1, 0, 0, 166, 1, 0, 0, 62, 0, 3, 0, 160, 1, 0, 0, 170, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 172, 1, 0, 0, 160, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 173, 1, 0, 0, 27, 1, 0, 0, 65, 0, 5, 0, 28, 1, 0, 0, 174, 1, 0, 0, 27, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 175, 1, 0, 0, 174, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 176, 1, 0, 0, 151, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 177, 1, 0, 0, 145, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 178, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 176, 1, 0, 0, 177, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 179, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 175, 1, 0, 0, 178, 1, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 180, 1, 0, 0, 179, 1, 0, 0, 179, 1, 0, 0, 179, 1, 0, 0, 179, 1, 0, 0, 12, 0, 8, 0, 9, 0, 0, 0, 181, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 172, 1, 0, 0, 173, 1, 0, 0, 180, 1, 0, 0, 62, 0, 3, 0, 171, 1, 0, 0, 181, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 183, 1, 0, 0, 171, 1, 0, 0, 79, 0, 8, 0, 111, 0, 0, 0, 184, 1, 0, 0, 183, 1, 0, 0, 183, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 185, 1, 0, 0, 171, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 186, 1, 0, 0, 185, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 187, 1, 0, 0, 145, 1, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 188, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 95, 0, 0, 0, 186, 1, 0, 0, 187, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 189, 1, 0, 0, 184, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 190, 1, 0, 0, 184, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 191, 1, 0, 0, 184, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 192, 1, 0, 0, 189, 1, 0, 0, 190, 1, 0, 0, 191, 1, 0, 0, 188, 1, 0, 0, 62, 0, 3, 0, 182, 1, 0, 0, 192, 1, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 193, 1, 0, 0, 182, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 194, 1, 0, 0, 193, 1, 0, 0, 184, 0, 5, 0, 151, 0, 0, 0, 196, 1, 0, 0, 194, 1, 0, 0, 195, 1, 0, 0, 247, 0, 3, 0, 198, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 196, 1, 0, 0, 197, 1, 0, 0, 198, 1, 0, 0, 248, 0, 2, 0, 197, 1, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 198, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 200, 1, 0, 0, 182, 1, 0, 0, 62, 0, 3, 0, 15, 1, 0, 0, 200, 1, 0, 0, 249, 0, 2, 0, 129, 1, 0, 0, 248, 0, 2, 0, 129, 1, 0, 0, 249, 0, 2, 0, 44, 1, 0, 0, 248, 0, 2, 0, 44, 1, 0, 0, 249, 0, 2, 0, 24, 1, 0, 0, 248, 0, 2, 0, 24, 1, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0, 54, 0, 5, 0, 9, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 13, 0, 0, 0, 55, 0, 3, 0, 10, 0, 0, 0, 14, 0, 0, 0, 55, 0, 3, 0, 10, 0, 0, 0, 15, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 16, 0, 0, 0, 248, 0, 2, 0, 18, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 37, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 42, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 50, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 66, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 80, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 98, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 38, 0, 0, 0, 13, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 41, 0, 0, 0, 38, 0, 0, 0, 40, 0, 0, 0, 62, 0, 3, 0, 37, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 43, 0, 0, 0, 37, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 44, 0, 0, 0, 37, 0, 0, 0, 148, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 43, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 42, 0, 0, 0, 45, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 47, 0, 0, 0, 16, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 39, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 49, 0, 0, 0, 39, 0, 0, 0, 48, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 49, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 51, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 52, 0, 0, 0, 46, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 53, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 46, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 56, 0, 0, 0, 54, 0, 0, 0, 55, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 57, 0, 0, 0, 53, 0, 0, 0, 56, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 58, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 59, 0, 0, 0, 46, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 60, 0, 0, 0, 58, 0, 0, 0, 59, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 46, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 62, 0, 0, 0, 61, 0, 0, 0, 55, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 63, 0, 0, 0, 60, 0, 0, 0, 62, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 64, 0, 0, 0, 57, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 50, 0, 0, 0, 64, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 39, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 65, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 68, 0, 0, 0, 65, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 69, 0, 0, 0, 67, 0, 0, 0, 68, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 70, 0, 0, 0, 65, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 55, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 72, 0, 0, 0, 69, 0, 0, 0, 71, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 73, 0, 0, 0, 65, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 65, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 75, 0, 0, 0, 73, 0, 0, 0, 74, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 0, 0, 0, 65, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 77, 0, 0, 0, 76, 0, 0, 0, 55, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 75, 0, 0, 0, 77, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 79, 0, 0, 0, 72, 0, 0, 0, 78, 0, 0, 0, 62, 0, 3, 0, 66, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 42, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 85, 0, 0, 0, 50, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 85, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 87, 0, 0, 0, 82, 0, 0, 0, 86, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 89, 0, 0, 0, 50, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 90, 0, 0, 0, 89, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 91, 0, 0, 0, 50, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 90, 0, 0, 0, 92, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 87, 0, 0, 0, 93, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 96, 0, 0, 0, 1, 0, 0, 0, 43, 0, 0, 0, 94, 0, 0, 0, 95, 0, 0, 0, 81, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 97, 0, 0, 0, 81, 0, 0, 0, 96, 0, 0, 0, 62, 0, 3, 0, 80, 0, 0, 0, 97, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 99, 0, 0, 0, 42, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 100, 0, 0, 0, 66, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 100, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 99, 0, 0, 0, 101, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 103, 0, 0, 0, 66, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 105, 0, 0, 0, 66, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 105, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 107, 0, 0, 0, 104, 0, 0, 0, 106, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 108, 0, 0, 0, 102, 0, 0, 0, 107, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 109, 0, 0, 0, 1, 0, 0, 0, 43, 0, 0, 0, 108, 0, 0, 0, 95, 0, 0, 0, 81, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 110, 0, 0, 0, 81, 0, 0, 0, 109, 0, 0, 0, 62, 0, 3, 0, 98, 0, 0, 0, 110, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 112, 0, 0, 0, 15, 0, 0, 0, 79, 0, 8, 0, 111, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 112, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 114, 0, 0, 0, 14, 0, 0, 0, 79, 0, 8, 0, 111, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 114, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 116, 0, 0, 0, 80, 0, 0, 0, 80, 0, 6, 0, 111, 0, 0, 0, 117, 0, 0, 0, 116, 0, 0, 0, 116, 0, 0, 0, 116, 0, 0, 0, 12, 0, 8, 0, 111, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 113, 0, 0, 0, 115, 0, 0, 0, 117, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 119, 0, 0, 0, 98, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 120, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 121, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 122, 0, 0, 0, 118, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 123, 0, 0, 0, 120, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 119, 0, 0, 0, 254, 0, 2, 0, 123, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 20, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 21, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 126, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 127, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 128, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 129, 0, 0, 0, 21, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 130, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 131, 0, 0, 0, 1, 0, 0, 0, 8, 0, 0, 0, 130, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 132, 0, 0, 0, 127, 0, 0, 0, 131, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 133, 0, 0, 0, 126, 0, 0, 0, 132, 0, 0, 0, 254, 0, 2, 0, 133, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 29, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 25, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 26, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 27, 0, 0, 0, 55, 0, 3, 0, 11, 0, 0, 0, 28, 0, 0, 0, 248, 0, 2, 0, 30, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 136, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 143, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 144, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 146, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 157, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 172, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 182, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 137, 0, 0, 0, 25, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 138, 0, 0, 0, 137, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 139, 0, 0, 0, 25, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 140, 0, 0, 0, 139, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 141, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 138, 0, 0, 0, 140, 0, 0, 0, 62, 0, 3, 0, 143, 0, 0, 0, 141, 0, 0, 0, 62, 0, 3, 0, 144, 0, 0, 0, 142, 0, 0, 0, 57, 0, 6, 0, 6, 0, 0, 0, 145, 0, 0, 0, 22, 0, 0, 0, 143, 0, 0, 0, 144, 0, 0, 0, 62, 0, 3, 0, 136, 0, 0, 0, 145, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 147, 0, 0, 0, 136, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 148, 0, 0, 0, 26, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 149, 0, 0, 0, 147, 0, 0, 0, 148, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 150, 0, 0, 0, 146, 0, 0, 0, 184, 0, 5, 0, 151, 0, 0, 0, 152, 0, 0, 0, 150, 0, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 154, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 152, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 248, 0, 2, 0, 153, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 155, 0, 0, 0, 146, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 156, 0, 0, 0, 155, 0, 0, 0, 142, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 156, 0, 0, 0, 249, 0, 2, 0, 154, 0, 0, 0, 248, 0, 2, 0, 154, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 158, 0, 0, 0, 27, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 159, 0, 0, 0, 26, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 158, 0, 0, 0, 159, 0, 0, 0, 62, 0, 3, 0, 157, 0, 0, 0, 160, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 161, 0, 0, 0, 157, 0, 0, 0, 184, 0, 5, 0, 151, 0, 0, 0, 162, 0, 0, 0, 161, 0, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 164, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 162, 0, 0, 0, 163, 0, 0, 0, 164, 0, 0, 0, 248, 0, 2, 0, 163, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 165, 0, 0, 0, 157, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 166, 0, 0, 0, 165, 0, 0, 0, 142, 0, 0, 0, 62, 0, 3, 0, 157, 0, 0, 0, 166, 0, 0, 0, 249, 0, 2, 0, 164, 0, 0, 0, 248, 0, 2, 0, 164, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 167, 0, 0, 0, 146, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 168, 0, 0, 0, 157, 0, 0, 0, 190, 0, 5, 0, 151, 0, 0, 0, 169, 0, 0, 0, 167, 0, 0, 0, 168, 0, 0, 0, 247, 0, 3, 0, 171, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 169, 0, 0, 0, 170, 0, 0, 0, 171, 0, 0, 0, 248, 0, 2, 0, 170, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 173, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 174, 0, 0, 0, 26, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 175, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 174, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 176, 0, 0, 0, 173, 0, 0, 0, 175, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 177, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 178, 0, 0, 0, 26, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 179, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 178, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 180, 0, 0, 0, 177, 0, 0, 0, 179, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 181, 0, 0, 0, 176, 0, 0, 0, 180, 0, 0, 0, 62, 0, 3, 0, 172, 0, 0, 0, 181, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 183, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 184, 0, 0, 0, 27, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 185, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 184, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 186, 0, 0, 0, 183, 0, 0, 0, 185, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 187, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 188, 0, 0, 0, 27, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 189, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 188, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 190, 0, 0, 0, 187, 0, 0, 0, 189, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 191, 0, 0, 0, 186, 0, 0, 0, 190, 0, 0, 0, 62, 0, 3, 0, 182, 0, 0, 0, 191, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 192, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 193, 0, 0, 0, 172, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 194, 0, 0, 0, 192, 0, 0, 0, 193, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 195, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 194, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 196, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 197, 0, 0, 0, 182, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 198, 0, 0, 0, 196, 0, 0, 0, 197, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 199, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 198, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 200, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 195, 0, 0, 0, 199, 0, 0, 0, 254, 0, 2, 0, 200, 0, 0, 0, 248, 0, 2, 0, 171, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 202, 0, 0, 0, 25, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 203, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 202, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 204, 0, 0, 0, 28, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 205, 0, 0, 0, 203, 0, 0, 0, 204, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 206, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 205, 0, 0, 0, 254, 0, 2, 0, 206, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 32, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 33, 0, 0, 0, 55, 0, 3, 0, 10, 0, 0, 0, 34, 0, 0, 0, 248, 0, 2, 0, 36, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 209, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 218, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 11, 0, 0, 0, 234, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 244, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 210, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 211, 0, 0, 0, 32, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 212, 0, 0, 0, 39, 0, 0, 0, 39, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 213, 0, 0, 0, 211, 0, 0, 0, 212, 0, 0, 0, 133, 0, 5, 0, 7, 0, 0, 0, 214, 0, 0, 0, 210, 0, 0, 0, 213, 0, 0, 0, 62, 0, 3, 0, 209, 0, 0, 0, 214, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 215, 0, 0, 0, 209, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 216, 0, 0, 0, 215, 0, 0, 0, 184, 0, 5, 0, 151, 0, 0, 0, 217, 0, 0, 0, 216, 0, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 220, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 217, 0, 0, 0, 219, 0, 0, 0, 223, 0, 0, 0, 248, 0, 2, 0, 219, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 221, 0, 0, 0, 34, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 222, 0, 0, 0, 221, 0, 0, 0, 221, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 62, 0, 3, 0, 218, 0, 0, 0, 222, 0, 0, 0, 249, 0, 2, 0, 220, 0, 0, 0, 248, 0, 2, 0, 223, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 224, 0, 0, 0, 34, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 225, 0, 0, 0, 224, 0, 0, 0, 224, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 62, 0, 3, 0, 218, 0, 0, 0, 225, 0, 0, 0, 249, 0, 2, 0, 220, 0, 0, 0, 248, 0, 2, 0, 220, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 226, 0, 0, 0, 218, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 227, 0, 0, 0, 34, 0, 0, 0, 88, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 228, 0, 0, 0, 226, 0, 0, 0, 0, 0, 0, 0, 62, 0, 3, 0, 227, 0, 0, 0, 228, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 229, 0, 0, 0, 34, 0, 0, 0, 84, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 230, 0, 0, 0, 226, 0, 0, 0, 1, 0, 0, 0, 62, 0, 3, 0, 229, 0, 0, 0, 230, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 231, 0, 0, 0, 209, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 232, 0, 0, 0, 231, 0, 0, 0, 184, 0, 5, 0, 151, 0, 0, 0, 233, 0, 0, 0, 232, 0, 0, 0, 95, 0, 0, 0, 247, 0, 3, 0, 236, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 233, 0, 0, 0, 235, 0, 0, 0, 239, 0, 0, 0, 248, 0, 2, 0, 235, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 237, 0, 0, 0, 34, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 238, 0, 0, 0, 237, 0, 0, 0, 62, 0, 3, 0, 234, 0, 0, 0, 238, 0, 0, 0, 249, 0, 2, 0, 236, 0, 0, 0, 248, 0, 2, 0, 239, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 240, 0, 0, 0, 34, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 241, 0, 0, 0, 240, 0, 0, 0, 62, 0, 3, 0, 234, 0, 0, 0, 241, 0, 0, 0, 249, 0, 2, 0, 236, 0, 0, 0, 248, 0, 2, 0, 236, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 242, 0, 0, 0, 234, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 243, 0, 0, 0, 34, 0, 0, 0, 88, 0, 0, 0, 62, 0, 3, 0, 243, 0, 0, 0, 242, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 245, 0, 0, 0, 209, 0, 0, 0, 12, 0, 6, 0, 7, 0, 0, 0, 246, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 247, 0, 0, 0, 33, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 248, 0, 0, 0, 247, 0, 0, 0, 39, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 249, 0, 0, 0, 246, 0, 0, 0, 248, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 250, 0, 0, 0, 34, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 251, 0, 0, 0, 250, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 252, 0, 0, 0, 251, 0, 0, 0, 251, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 253, 0, 0, 0, 249, 0, 0, 0, 252, 0, 0, 0, 62, 0, 3, 0, 244, 0, 0, 0, 253, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 254, 0, 0, 0, 244, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 255, 0, 0, 0, 254, 0, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 0, 1, 0, 0, 244, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 2, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 255, 0, 0, 0, 1, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 3, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 2, 1, 0, 0, 95, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 4, 1, 0, 0, 244, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 5, 1, 0, 0, 95, 0, 0, 0, 95, 0, 0, 0, 12, 0, 7, 0, 7, 0, 0, 0, 6, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 4, 1, 0, 0, 5, 1, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 7, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 6, 1, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 8, 1, 0, 0, 3, 1, 0, 0, 7, 1, 0, 0, 65, 0, 5, 0, 11, 0, 0, 0, 9, 1, 0, 0, 34, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 11, 1, 0, 0, 8, 1, 0, 0, 10, 1, 0, 0, 254, 0, 2, 0, 11, 1, 0, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SHAPE_VERT[4736] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 162, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 23, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 18, 0, 0, 0, 53, 0, 0, 0, 56, 0, 0, 0, 101, 0, 0, 0, 123, 0, 0, 0, 124, 0, 0, 0, 127, 0, 0, 0, 129, 0, 0, 0, 131, 0, 0, 0, 133, 0, 0, 0, 134, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 140, 0, 0, 0, 141, 0, 0, 0, 144, 0, 0, 0, 145, 0, 0, 0, 150, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 10, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 18, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 51, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 53, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 56, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 99, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 101, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 105, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 110, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 110, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 110, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 110, 0, 0, 0, 2, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 110, 0, 0, 0, 3, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 110, 0, 0, 0, 4, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 112, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 123, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 124, 0, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 127, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 129, 0, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 131, 0, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 133, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 6, 0, 134, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 7, 0, 137, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 7, 0, 138, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 6, 0, 140, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 6, 0, 141, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 4, 0, 144, 0, 0, 0, 118, 95, 115, 104, 97, 112, 101, 0, 5, 0, 4, 0, 145, 0, 0, 0, 105, 95, 115, 104, 97, 112, 101, 0, 5, 0, 6, 0, 148, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 148, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 148, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 148, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 148, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 150, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 53, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 56, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 101, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 200, 0, 0, 0, 71, 0, 3, 0, 110, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 112, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 112, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 123, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 124, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 127, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 127, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 129, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 131, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 131, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 133, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 133, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 134, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 137, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 137, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 138, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 140, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 140, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 141, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 144, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 144, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 145, 0, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 72, 0, 5, 0, 148, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 148, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 148, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 148, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 148, 0, 0, 0, 2, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 9, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 11, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 13, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 14, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 15, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 23, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 19, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 49, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 50, 0, 0, 0, 7, 0, 0, 0, 49, 0, 0, 0, 32, 0, 4, 0, 52, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 53, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 56, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 59, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 60, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 61, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 62, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 65, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 69, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 76, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 97, 0, 0, 0, 32, 0, 4, 0, 98, 0, 0, 0, 7, 0, 0, 0, 97, 0, 0, 0, 32, 0, 4, 0, 100, 0, 0, 0, 1, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 100, 0, 0, 0, 101, 0, 0, 0, 1, 0, 0, 0, 30, 0, 7, 0, 110, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 49, 0, 0, 0, 49, 0, 0, 0, 32, 0, 4, 0, 111, 0, 0, 0, 2, 0, 0, 0, 110, 0, 0, 0, 59, 0, 4, 0, 111, 0, 0, 0, 112, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 113, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 32, 0, 4, 0, 122, 0, 0, 0, 3, 0, 0, 0, 49, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 123, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 126, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 126, 0, 0, 0, 127, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 128, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 128, 0, 0, 0, 129, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 131, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 126, 0, 0, 0, 133, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 128, 0, 0, 0, 134, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 136, 0, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 136, 0, 0, 0, 137, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 21, 0, 0, 0, 138, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 126, 0, 0, 0, 140, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 128, 0, 0, 0, 141, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 143, 0, 0, 0, 3, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 143, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 100, 0, 0, 0, 145, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 147, 0, 0, 0, 6, 0, 0, 0, 24, 0, 0, 0, 30, 0, 6, 0, 148, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 147, 0, 0, 0, 147, 0, 0, 0, 32, 0, 4, 0, 149, 0, 0, 0, 3, 0, 0, 0, 148, 0, 0, 0, 59, 0, 4, 0, 149, 0, 0, 0, 150, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 158, 0, 0, 0, 2, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 10, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 50, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 98, 0, 0, 0, 99, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 105, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 107, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 22, 0, 0, 0, 18, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 25, 0, 0, 0, 18, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 27, 0, 0, 0, 23, 0, 0, 0, 26, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 28, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 29, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 30, 0, 0, 0, 13, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 31, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 32, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 14, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 35, 0, 0, 0, 14, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 37, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 38, 0, 0, 0, 15, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 15, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 40, 0, 0, 0, 27, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 41, 0, 0, 0, 27, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 42, 0, 0, 0, 27, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 43, 0, 0, 0, 27, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 44, 0, 0, 0, 28, 0, 0, 0, 29, 0, 0, 0, 30, 0, 0, 0, 31, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 45, 0, 0, 0, 32, 0, 0, 0, 33, 0, 0, 0, 34, 0, 0, 0, 35, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 46, 0, 0, 0, 36, 0, 0, 0, 37, 0, 0, 0, 38, 0, 0, 0, 39, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 47, 0, 0, 0, 40, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 43, 0, 0, 0, 80, 0, 7, 0, 8, 0, 0, 0, 48, 0, 0, 0, 44, 0, 0, 0, 45, 0, 0, 0, 46, 0, 0, 0, 47, 0, 0, 0, 62, 0, 3, 0, 10, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 127, 0, 4, 0, 49, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 49, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 63, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 65, 0, 5, 0, 65, 0, 0, 0, 66, 0, 0, 0, 51, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 68, 0, 0, 0, 64, 0, 0, 0, 67, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 70, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 65, 0, 5, 0, 65, 0, 0, 0, 72, 0, 0, 0, 51, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 73, 0, 0, 0, 72, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 74, 0, 0, 0, 71, 0, 0, 0, 73, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 75, 0, 0, 0, 68, 0, 0, 0, 74, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 77, 0, 0, 0, 10, 0, 0, 0, 76, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 79, 0, 0, 0, 78, 0, 0, 0, 12, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 80, 0, 0, 0, 75, 0, 0, 0, 79, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 81, 0, 0, 0, 10, 0, 0, 0, 60, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 82, 0, 0, 0, 81, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 83, 0, 0, 0, 80, 0, 0, 0, 82, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 84, 0, 0, 0, 10, 0, 0, 0, 60, 0, 0, 0, 62, 0, 3, 0, 84, 0, 0, 0, 83, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 85, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 86, 0, 0, 0, 85, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 87, 0, 0, 0, 56, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 88, 0, 0, 0, 87, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 89, 0, 0, 0, 86, 0, 0, 0, 88, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 90, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 62, 0, 3, 0, 90, 0, 0, 0, 89, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 91, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 93, 0, 0, 0, 56, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 94, 0, 0, 0, 93, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 95, 0, 0, 0, 92, 0, 0, 0, 94, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 96, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 96, 0, 0, 0, 95, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 102, 0, 0, 0, 101, 0, 0, 0, 199, 0, 5, 0, 19, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 24, 0, 0, 0, 170, 0, 5, 0, 97, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 24, 0, 0, 0, 62, 0, 3, 0, 99, 0, 0, 0, 104, 0, 0, 0, 61, 0, 4, 0, 97, 0, 0, 0, 106, 0, 0, 0, 99, 0, 0, 0, 247, 0, 3, 0, 109, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 106, 0, 0, 0, 108, 0, 0, 0, 116, 0, 0, 0, 248, 0, 2, 0, 108, 0, 0, 0, 65, 0, 5, 0, 113, 0, 0, 0, 114, 0, 0, 0, 112, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 62, 0, 3, 0, 107, 0, 0, 0, 115, 0, 0, 0, 249, 0, 2, 0, 109, 0, 0, 0, 248, 0, 2, 0, 116, 0, 0, 0, 65, 0, 5, 0, 113, 0, 0, 0, 117, 0, 0, 0, 112, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 118, 0, 0, 0, 117, 0, 0, 0, 62, 0, 3, 0, 107, 0, 0, 0, 118, 0, 0, 0, 249, 0, 2, 0, 109, 0, 0, 0, 248, 0, 2, 0, 109, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 119, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 120, 0, 0, 0, 10, 0, 0, 0, 146, 0, 5, 0, 8, 0, 0, 0, 121, 0, 0, 0, 119, 0, 0, 0, 120, 0, 0, 0, 62, 0, 3, 0, 105, 0, 0, 0, 121, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 125, 0, 0, 0, 124, 0, 0, 0, 62, 0, 3, 0, 123, 0, 0, 0, 125, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 130, 0, 0, 0, 129, 0, 0, 0, 62, 0, 3, 0, 127, 0, 0, 0, 130, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 132, 0, 0, 0, 56, 0, 0, 0, 62, 0, 3, 0, 131, 0, 0, 0, 132, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 135, 0, 0, 0, 134, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 135, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 139, 0, 0, 0, 138, 0, 0, 0, 62, 0, 3, 0, 137, 0, 0, 0, 139, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 142, 0, 0, 0, 141, 0, 0, 0, 62, 0, 3, 0, 140, 0, 0, 0, 142, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 146, 0, 0, 0, 145, 0, 0, 0, 62, 0, 3, 0, 144, 0, 0, 0, 146, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 151, 0, 0, 0, 105, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 152, 0, 0, 0, 124, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 153, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 154, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 155, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 145, 0, 5, 0, 7, 0, 0, 0, 156, 0, 0, 0, 151, 0, 0, 0, 155, 0, 0, 0, 65, 0, 5, 0, 126, 0, 0, 0, 157, 0, 0, 0, 150, 0, 0, 0, 61, 0, 0, 0, 62, 0, 3, 0, 157, 0, 0, 0, 156, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 159, 0, 0, 0, 18, 0, 0, 0, 158, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 65, 0, 6, 0, 136, 0, 0, 0, 161, 0, 0, 0, 150, 0, 0, 0, 61, 0, 0, 0, 158, 0, 0, 0, 62, 0, 3, 0, 161, 0, 0, 0, 160, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SPRITE_FRAG[3524] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 163, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 10, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 14, 0, 0, 0, 35, 0, 0, 0, 140, 0, 0, 0, 161, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 9, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 14, 0, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 4, 0, 22, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 5, 0, 25, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 29, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 35, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 3, 0, 133, 0, 0, 0, 99, 0, 0, 0, 5, 0, 6, 0, 140, 0, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 161, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 14, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 25, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 25, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 29, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 29, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 35, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 140, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 140, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 161, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 13, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 13, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 20, 0, 2, 0, 17, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 25, 0, 9, 0, 23, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 24, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 59, 0, 4, 0, 24, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 27, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 29, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 31, 0, 0, 0, 23, 0, 0, 0, 23, 0, 4, 0, 33, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 34, 0, 0, 0, 1, 0, 0, 0, 33, 0, 0, 0, 59, 0, 4, 0, 34, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 41, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 41, 0, 0, 0, 42, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 131, 0, 0, 0, 0, 0, 128, 63, 59, 0, 4, 0, 10, 0, 0, 0, 140, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 155, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 160, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 160, 0, 0, 0, 161, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 21, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 133, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 15, 0, 0, 0, 14, 0, 0, 0, 186, 0, 5, 0, 17, 0, 0, 0, 18, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 247, 0, 3, 0, 20, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 18, 0, 0, 0, 19, 0, 0, 0, 145, 0, 0, 0, 248, 0, 2, 0, 19, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 26, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 37, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 16, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 40, 0, 0, 0, 32, 0, 0, 0, 39, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 43, 0, 0, 0, 40, 0, 0, 0, 3, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 43, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 44, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 45, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 46, 0, 0, 0, 44, 0, 0, 0, 45, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 47, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 49, 0, 0, 0, 48, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 16, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 51, 0, 0, 0, 47, 0, 0, 0, 50, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 52, 0, 0, 0, 46, 0, 0, 0, 51, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 55, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 56, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 57, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 58, 0, 0, 0, 56, 0, 0, 0, 57, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 59, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 60, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 61, 0, 0, 0, 16, 0, 0, 0, 60, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 62, 0, 0, 0, 59, 0, 0, 0, 61, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 63, 0, 0, 0, 58, 0, 0, 0, 62, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 65, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 66, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 66, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 67, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 68, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 69, 0, 0, 0, 67, 0, 0, 0, 68, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 70, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 71, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 72, 0, 0, 0, 71, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 73, 0, 0, 0, 16, 0, 0, 0, 72, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 74, 0, 0, 0, 70, 0, 0, 0, 73, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 75, 0, 0, 0, 69, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 76, 0, 0, 0, 75, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 76, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 78, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 79, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 80, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 81, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 82, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 83, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 84, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 84, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 86, 0, 0, 0, 83, 0, 0, 0, 85, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 87, 0, 0, 0, 82, 0, 0, 0, 86, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 88, 0, 0, 0, 81, 0, 0, 0, 87, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 89, 0, 0, 0, 88, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 90, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 91, 0, 0, 0, 90, 0, 0, 0, 89, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 91, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 93, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 94, 0, 0, 0, 92, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 95, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 97, 0, 0, 0, 96, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 98, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 99, 0, 0, 0, 97, 0, 0, 0, 98, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 100, 0, 0, 0, 95, 0, 0, 0, 99, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 101, 0, 0, 0, 94, 0, 0, 0, 100, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 101, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 103, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 104, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 105, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 106, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 108, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 109, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 111, 0, 0, 0, 108, 0, 0, 0, 110, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 112, 0, 0, 0, 107, 0, 0, 0, 111, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 113, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 115, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 116, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 117, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 118, 0, 0, 0, 116, 0, 0, 0, 117, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 119, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 120, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 122, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 123, 0, 0, 0, 122, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 124, 0, 0, 0, 121, 0, 0, 0, 123, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 125, 0, 0, 0, 119, 0, 0, 0, 124, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 126, 0, 0, 0, 118, 0, 0, 0, 125, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 126, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 128, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 128, 0, 0, 0, 127, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 129, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 130, 0, 0, 0, 22, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 132, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 132, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 134, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 135, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 136, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 137, 0, 0, 0, 35, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 138, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 138, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 139, 0, 0, 0, 133, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 141, 0, 0, 0, 140, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 142, 0, 0, 0, 22, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 143, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 12, 0, 8, 0, 7, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 139, 0, 0, 0, 141, 0, 0, 0, 143, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 144, 0, 0, 0, 249, 0, 2, 0, 20, 0, 0, 0, 248, 0, 2, 0, 145, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 146, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 147, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 148, 0, 0, 0, 146, 0, 0, 0, 147, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 149, 0, 0, 0, 35, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 150, 0, 0, 0, 148, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 151, 0, 0, 0, 11, 0, 0, 0, 133, 0, 5, 0, 7, 0, 0, 0, 152, 0, 0, 0, 150, 0, 0, 0, 151, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 152, 0, 0, 0, 249, 0, 2, 0, 20, 0, 0, 0, 248, 0, 2, 0, 20, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 153, 0, 0, 0, 9, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 154, 0, 0, 0, 153, 0, 0, 0, 184, 0, 5, 0, 17, 0, 0, 0, 156, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 247, 0, 3, 0, 158, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 156, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 248, 0, 2, 0, 157, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 158, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 162, 0, 0, 0, 9, 0, 0, 0, 62, 0, 3, 0, 161, 0, 0, 0, 162, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SPRITE_VERT[7752] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 41, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 20, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 147, 0, 0, 0, 181, 0, 0, 0, 184, 0, 0, 0, 228, 0, 0, 0, 6, 1, 0, 0, 7, 1, 0, 0, 9, 1, 0, 0, 17, 1, 0, 0, 18, 1, 0, 0, 20, 1, 0, 0, 21, 1, 0, 0, 24, 1, 0, 0, 25, 1, 0, 0, 30, 1, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 3, 0, 8, 0, 0, 0, 113, 120, 120, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 27, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 34, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 40, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 46, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 52, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 59, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 65, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 73, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 5, 0, 141, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 147, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 179, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 181, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 184, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 226, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 228, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 7, 0, 232, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 3, 0, 236, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 241, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 241, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 9, 0, 241, 0, 0, 0, 2, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 3, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 241, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 243, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 6, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 7, 1, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 7, 0, 9, 1, 0, 0, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 4, 0, 17, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 18, 1, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 20, 1, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 21, 1, 0, 0, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 24, 1, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 7, 0, 25, 1, 0, 0, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 6, 0, 28, 1, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 28, 1, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 28, 1, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 28, 1, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 28, 1, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 30, 1, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 147, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 181, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 184, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 228, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 3, 0, 241, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 6, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 9, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 17, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 17, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 18, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 20, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 20, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 21, 1, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 24, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 24, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 25, 1, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 28, 1, 0, 0, 2, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 12, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 53, 0, 0, 0, 3, 0, 0, 0, 24, 0, 4, 0, 71, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 72, 0, 0, 0, 7, 0, 0, 0, 71, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 119, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 142, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 143, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 144, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 23, 0, 4, 0, 145, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 146, 0, 0, 0, 1, 0, 0, 0, 145, 0, 0, 0, 59, 0, 4, 0, 146, 0, 0, 0, 147, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 177, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 178, 0, 0, 0, 7, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 180, 0, 0, 0, 1, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 181, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 187, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 188, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 190, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 196, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 203, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 224, 0, 0, 0, 32, 0, 4, 0, 225, 0, 0, 0, 7, 0, 0, 0, 224, 0, 0, 0, 32, 0, 4, 0, 227, 0, 0, 0, 1, 0, 0, 0, 12, 0, 0, 0, 59, 0, 4, 0, 227, 0, 0, 0, 228, 0, 0, 0, 1, 0, 0, 0, 30, 0, 10, 0, 241, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 177, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 242, 0, 0, 0, 2, 0, 0, 0, 241, 0, 0, 0, 59, 0, 4, 0, 242, 0, 0, 0, 243, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 244, 0, 0, 0, 2, 0, 0, 0, 71, 0, 0, 0, 32, 0, 4, 0, 5, 1, 0, 0, 3, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 6, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 7, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 9, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 16, 1, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 17, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 18, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 20, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 21, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 23, 1, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 23, 1, 0, 0, 24, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 25, 1, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 27, 1, 0, 0, 6, 0, 0, 0, 21, 0, 0, 0, 30, 0, 6, 0, 28, 1, 0, 0, 9, 0, 0, 0, 6, 0, 0, 0, 27, 1, 0, 0, 27, 1, 0, 0, 32, 0, 4, 0, 29, 1, 0, 0, 3, 0, 0, 0, 28, 1, 0, 0, 59, 0, 4, 0, 29, 1, 0, 0, 30, 1, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 27, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 34, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 59, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 73, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 141, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 178, 0, 0, 0, 179, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 226, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 232, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 236, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 238, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 249, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 15, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 17, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 18, 0, 0, 0, 17, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 19, 0, 0, 0, 16, 0, 0, 0, 18, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 22, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 24, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 25, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 29, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 31, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 31, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 30, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 27, 0, 0, 0, 33, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 35, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 37, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 34, 0, 0, 0, 39, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 41, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 43, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 42, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 45, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 47, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 49, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 51, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 54, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 56, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 60, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 62, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 59, 0, 0, 0, 64, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 66, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 68, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 70, 0, 0, 0, 67, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 79, 0, 0, 0, 75, 0, 0, 0, 78, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 80, 0, 0, 0, 74, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 81, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 65, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 84, 0, 0, 0, 75, 0, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 59, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 87, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 88, 0, 0, 0, 75, 0, 0, 0, 87, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 90, 0, 0, 0, 80, 0, 0, 0, 84, 0, 0, 0, 88, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 65, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 91, 0, 0, 0, 92, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 75, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 97, 0, 0, 0, 95, 0, 0, 0, 96, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 98, 0, 0, 0, 75, 0, 0, 0, 97, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 99, 0, 0, 0, 74, 0, 0, 0, 98, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 100, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 52, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 103, 0, 0, 0, 75, 0, 0, 0, 102, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 104, 0, 0, 0, 94, 0, 0, 0, 99, 0, 0, 0, 103, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 105, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 59, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 108, 0, 0, 0, 75, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 111, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 112, 0, 0, 0, 75, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 113, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 20, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 114, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 116, 0, 0, 0, 75, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 117, 0, 0, 0, 74, 0, 0, 0, 116, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 118, 0, 0, 0, 108, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 89, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 120, 0, 0, 0, 90, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 121, 0, 0, 0, 90, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 122, 0, 0, 0, 90, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 123, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 124, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 125, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 126, 0, 0, 0, 104, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 104, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 128, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 130, 0, 0, 0, 118, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 131, 0, 0, 0, 118, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 132, 0, 0, 0, 119, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 133, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 134, 0, 0, 0, 119, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 135, 0, 0, 0, 119, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 136, 0, 0, 0, 120, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 123, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 137, 0, 0, 0, 124, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 138, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 139, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 140, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 139, 0, 0, 0, 62, 0, 3, 0, 73, 0, 0, 0, 140, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 150, 0, 0, 0, 147, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 152, 0, 0, 0, 149, 0, 0, 0, 151, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 153, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 154, 0, 0, 0, 142, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 155, 0, 0, 0, 142, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 156, 0, 0, 0, 142, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 157, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 158, 0, 0, 0, 143, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 159, 0, 0, 0, 143, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 143, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 161, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 162, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 163, 0, 0, 0, 144, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 164, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 165, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 166, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 167, 0, 0, 0, 152, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 168, 0, 0, 0, 152, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 169, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 156, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 170, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 159, 0, 0, 0, 160, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 171, 0, 0, 0, 161, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 164, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 172, 0, 0, 0, 165, 0, 0, 0, 166, 0, 0, 0, 167, 0, 0, 0, 168, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 173, 0, 0, 0, 169, 0, 0, 0, 170, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 173, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 174, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 175, 0, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 176, 0, 0, 0, 175, 0, 0, 0, 174, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 176, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 182, 0, 0, 0, 181, 0, 0, 0, 127, 0, 4, 0, 177, 0, 0, 0, 183, 0, 0, 0, 182, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 185, 0, 0, 0, 184, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 186, 0, 0, 0, 183, 0, 0, 0, 185, 0, 0, 0, 62, 0, 3, 0, 179, 0, 0, 0, 186, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 191, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 192, 0, 0, 0, 191, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 193, 0, 0, 0, 179, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 194, 0, 0, 0, 193, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 195, 0, 0, 0, 192, 0, 0, 0, 194, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 197, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 198, 0, 0, 0, 197, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 199, 0, 0, 0, 179, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 200, 0, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 201, 0, 0, 0, 198, 0, 0, 0, 200, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 202, 0, 0, 0, 195, 0, 0, 0, 201, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 204, 0, 0, 0, 141, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 89, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 207, 0, 0, 0, 202, 0, 0, 0, 206, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 208, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 209, 0, 0, 0, 208, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 210, 0, 0, 0, 207, 0, 0, 0, 209, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 211, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 62, 0, 3, 0, 211, 0, 0, 0, 210, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 212, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 213, 0, 0, 0, 212, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 214, 0, 0, 0, 184, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 215, 0, 0, 0, 214, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 216, 0, 0, 0, 213, 0, 0, 0, 215, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 217, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 217, 0, 0, 0, 216, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 218, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 219, 0, 0, 0, 218, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 220, 0, 0, 0, 184, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 221, 0, 0, 0, 220, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 222, 0, 0, 0, 219, 0, 0, 0, 221, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 223, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 62, 0, 3, 0, 223, 0, 0, 0, 222, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 229, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 230, 0, 0, 0, 229, 0, 0, 0, 21, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 231, 0, 0, 0, 230, 0, 0, 0, 21, 0, 0, 0, 62, 0, 3, 0, 226, 0, 0, 0, 231, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 233, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 28, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 232, 0, 0, 0, 235, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 237, 0, 0, 0, 226, 0, 0, 0, 247, 0, 3, 0, 240, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 237, 0, 0, 0, 239, 0, 0, 0, 247, 0, 0, 0, 248, 0, 2, 0, 239, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 245, 0, 0, 0, 243, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 246, 0, 0, 0, 245, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 246, 0, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 247, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 248, 0, 0, 0, 232, 0, 0, 0, 247, 0, 3, 0, 251, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 248, 0, 0, 0, 250, 0, 0, 0, 254, 0, 0, 0, 248, 0, 2, 0, 250, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 252, 0, 0, 0, 243, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 253, 0, 0, 0, 252, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 253, 0, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 254, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 255, 0, 0, 0, 243, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 0, 1, 0, 0, 255, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 0, 1, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 251, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 1, 1, 0, 0, 249, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 1, 1, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 240, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 2, 1, 0, 0, 238, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 3, 1, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 4, 1, 0, 0, 2, 1, 0, 0, 3, 1, 0, 0, 62, 0, 3, 0, 236, 0, 0, 0, 4, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 10, 1, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 12, 1, 0, 0, 8, 1, 0, 0, 11, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 13, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 14, 1, 0, 0, 13, 1, 0, 0, 13, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 177, 0, 0, 0, 15, 1, 0, 0, 12, 1, 0, 0, 14, 1, 0, 0, 62, 0, 3, 0, 6, 1, 0, 0, 15, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 19, 1, 0, 0, 18, 1, 0, 0, 62, 0, 3, 0, 17, 1, 0, 0, 19, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 22, 1, 0, 0, 21, 1, 0, 0, 62, 0, 3, 0, 20, 1, 0, 0, 22, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 26, 1, 0, 0, 25, 1, 0, 0, 62, 0, 3, 0, 24, 1, 0, 0, 26, 1, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 31, 1, 0, 0, 236, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 32, 1, 0, 0, 7, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 33, 1, 0, 0, 32, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 34, 1, 0, 0, 32, 1, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 35, 1, 0, 0, 33, 1, 0, 0, 34, 1, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 145, 0, 5, 0, 9, 0, 0, 0, 36, 1, 0, 0, 31, 1, 0, 0, 35, 1, 0, 0, 65, 0, 5, 0, 16, 1, 0, 0, 37, 1, 0, 0, 30, 1, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 37, 1, 0, 0, 36, 1, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 38, 1, 0, 0, 147, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 39, 1, 0, 0, 38, 1, 0, 0, 65, 0, 6, 0, 23, 1, 0, 0, 40, 1, 0, 0, 30, 1, 0, 0, 189, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 40, 1, 0, 0, 39, 1, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

#endif