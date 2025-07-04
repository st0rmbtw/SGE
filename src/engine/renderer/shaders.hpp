#ifndef _SGE_RENDERER_SHADERS_HPP_
#define _SGE_RENDERER_SHADERS_HPP_

#include <cstdlib>
#include <SGE/types/backend.hpp>
#include <SGE/assert.hpp>

static const char D3D11_FONT[1541] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
const float dist = Texture.Sample(Sampler, inp.uv).r;
const float width = fwidth(dist);
const float alpha = smoothstep(GLYPH_CENTER - width, GLYPH_CENTER + width, abs(dist));
const float4 color = float4(inp.color, alpha);
clip(color.a - 0.05f);
return color;
};
)";

static const char D3D11_LINE[2411] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
float2 i_start : I_Start;
float2 i_end : I_End;
float4 i_color : I_Color;
float4 i_border_radius : I_Border_Radius;
float i_thickness : I_Thickness;
uint i_flags : I_Flags;
uint vid : SV_VertexID;
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
const float2 vertices[4] = {
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
outp.position = mul(mvp, float4(vertices[inp.vid], 0.0, 1.0));
outp.position.z = 1.0;
return outp;
}
float sd_rounded_box(float2 p, float2 size, float4 corner_radii) {
float2 rs = 0.0 < p.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < p.x ? rs.y : rs.x;
float2 corner_to_point = abs(p) - 0.5 * size;
float2 q = corner_to_point + radius;
float l = length(max(q, float2(0.0, 0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
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
})";

static const char D3D11_NINEPATCH[4570] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
transform[0][3] = transform[0][0] * offset[0] + transform[0][1] * offset[1] + transform[0][2] * 0.0 + transform[0][3];
transform[1][3] = transform[1][0] * offset[0] + transform[1][1] * offset[1] + transform[1][2] * 0.0 + transform[1][3];
transform[2][3] = transform[2][0] * offset[0] + transform[2][1] * offset[1] + transform[2][2] * 0.0 + transform[2][3];
transform[3][3] = transform[3][0] * offset[0] + transform[3][1] * offset[1] + transform[3][2] * 0.0 + transform[3][3];
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

static const char D3D11_SHAPE[6814] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
float2 p  : Point;
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
float4x4 transform = float4x4(
float4(1.0, 0.0, 0.0, inp.i_position.x),
float4(0.0, 1.0, 0.0, inp.i_position.y),
float4(0.0, 0.0, 1.0, 0.0),
float4(0.0, 0.0, 0.0, 1.0)
);
const float2 offset = -inp.i_offset * inp.i_size;
transform[0][3] = transform[0][0] * offset[0] + transform[0][1] * offset[1] + transform[0][2] * 0.0 + transform[0][3];
transform[1][3] = transform[1][0] * offset[0] + transform[1][1] * offset[1] + transform[1][2] * 0.0 + transform[1][3];
transform[2][3] = transform[2][0] * offset[0] + transform[2][1] * offset[1] + transform[2][2] * 0.0 + transform[2][3];
transform[3][3] = transform[3][0] * offset[0] + transform[3][1] * offset[1] + transform[3][2] * 0.0 + transform[3][3];
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
outp.p = (position - 0.5) * inp.i_size;
outp.size = inp.i_size;
outp.color = inp.i_color;
outp.border_color = inp.i_border_color;
outp.border_thickness = inp.i_border_thickness;
outp.border_radius = inp.i_border_radius;
outp.shape = inp.i_shape;
return outp;
}
float sdf_circle(float2 p, float radius) {
return length(p) - radius;
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
if (ap >= a1p) {
float2 q0 = float2(r * cos(a0), r * sin(a0));
float2 q1 = float2(r * cos(a1), r * sin(a1));
return min(length(p - q0), length(p - q1));
}
return abs(length(p) - r);
}
float sd_rounded_box(float2 p, float2 size, float4 corner_radii) {
float2 rs = 0.0 < p.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < p.x ? rs.y : rs.x;
float2 corner_to_point = abs(p) - 0.5 * size;
float2 q = corner_to_point + radius;
float l = length(max(q, float2(0.0, 0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
float sd_inset_rounded_box(float2 p, float2 size, float4 radius, float4 inset) {
float2 inner_size = size - inset.xy - inset.zw;
float2 inner_center = inset.xy + 0.5 * inner_size - 0.5 * size;
float2 inner_point = p - inner_center;
float4 r = radius;
r.x = r.x - max(inset.x, inset.y);
r.y = r.y - max(inset.z, inset.y);
r.z = r.z - max(inset.z, inset.w);
r.w = r.w - max(inset.x, inset.w);
float2 half_size = inner_size * 0.5;
float min_size = min(half_size.x, half_size.y);
r = min(max(r, float4(0.0, 0.0, 0.0, 0.0)), float4(min_size, min_size, min_size, min_size));
return sd_rounded_box(inner_point, inner_size, r);
}
float antialias(float d) {
float afwidth = pow(fwidth(d), 1.0 / 2.2);
return 1.0 - smoothstep(0.0, afwidth, d);
}
float antialias_circle(float d) {
float afwidth = fwidth(d) * 1.3;
return 1.0 - smoothstep(0.0, afwidth, d);
}
float4 PS(VSOutput inp) : SV_Target
{
float4 result = inp.color;
if (inp.shape == SHAPE_CIRCLE) {
float radius = 0.5 - 1.0 / inp.size.x;
float external_distance = sdf_circle(inp.uv - 0.5, radius);
float alpha = antialias_circle(external_distance);
if (inp.border_thickness > 0.0) {
float internal_distance = sdf_circle(inp.uv - 0.5, radius - inp.border_thickness / inp.size.x);
float border_distance = max(external_distance, -internal_distance);
float border_alpha = antialias_circle(border_distance);
float4 color = float4(inp.color.rgb, min(inp.color.a, alpha));
float4 color_with_border = lerp(color, inp.border_color, min(inp.border_color.a, min(border_alpha, alpha)));
result = internal_distance > 0.0 && border_alpha < 1.0
? lerp(float4(0.0, 0.0, 0.0, 0.0), inp.border_color, border_alpha)
: color_with_border;
} else {
result = float4(inp.color.rgb, min(inp.color.a, alpha));
}
} else if (inp.shape == SHAPE_ARC) {
float start_angle = inp.border_radius.x;
float end_angle = inp.border_radius.y;
float thickness = 0.5 - inp.border_thickness / inp.size.y;
float2 p = ((float2(inp.uv.x, 1.0 - inp.uv.y) * 2.0 - 1.0) * inp.size) / inp.size.y;
float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
float aa = length(float2(ddx(d), ddy(d)));
float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);
return float4(inp.color.rgb, min(inp.color.a, alpha));
} else {
float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));
if (radius > 0.0 || inp.border_thickness > 0.0) {
float external_distance = sd_rounded_box(inp.p, inp.size, inp.border_radius);
float internal_distance = sd_inset_rounded_box(inp.p, inp.size, inp.border_radius, float4(inp.border_thickness, inp.border_thickness, inp.border_thickness, inp.border_thickness));
float border_distance = max(external_distance, -internal_distance);
float border_alpha = antialias(border_distance);
float smoothed_alpha = antialias(external_distance);
float4 quad_color = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
float4 quad_color_with_border = lerp(quad_color, inp.border_color, min(inp.border_color.a, min(border_alpha, smoothed_alpha)));
result = internal_distance > 0.0 && border_alpha < 1.0
? float4(inp.border_color.rgb, border_alpha)
: quad_color_with_border;
}
}
clip(result.a - 0.01);
return result;
})";

static const char D3D11_SPRITE[4814] = R"(cbuffer GlobalUniformBuffer : register( b2 )
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
transform[0][3] = transform[0][0] * offset[0] + transform[0][1] * offset[1] + transform[0][2] * 0.0 + transform[0][3];
transform[1][3] = transform[1][0] * offset[0] + transform[1][1] * offset[1] + transform[1][2] * 0.0 + transform[1][3];
transform[2][3] = transform[2][0] * offset[0] + transform[2][1] * offset[1] + transform[2][2] * 0.0 + transform[2][3];
transform[3][3] = transform[3][0] * offset[0] + transform[3][1] * offset[1] + transform[3][2] * 0.0 + transform[3][3];
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
clip(color.a - 0.025);
return color;
};
)";

static const char METAL_FONT[1710] = R"(#include <metal_stdlib>
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
const float dist = texture.sample(texture_sampler, inp.uv).r;
const float width = fwidth(dist);
const float alpha = smoothstep(GLYPH_CENTER - width, GLYPH_CENTER + width, abs(dist));
const float4 color = float4(inp.color, alpha);
if (color.a < 0.05) discard_fragment();
return color;
})";

static const char METAL_LINE[2558] = R"(#include <metal_stdlib>
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
float2 position        [[attribute(0)]];
float2 i_start         [[attribute(1)]];
float2 i_end           [[attribute(2)]];
float4 i_color         [[attribute(3)]];
float4 i_border_radius [[attribute(4)]];
float  i_thickness     [[attribute(5)]];
uint   i_flags         [[attribute(6)]];
};
struct VertexOut
{
float4 position [[position]];
float4 color [[flat]];
float4 border_radius [[flat]];
float2 size [[flat]];
float2 point;
float2 uv;
};
constant constexpr uint FLAG_UI = 1 << 0;
vertex VertexOut VS(
VertexIn inp [[stage_in]],
constant Constants& constants [[buffer(2)]],
uint vid [[vertex_id]]
) {
const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;
const float4x4 mvp = is_ui ? constants.screen_projection : constants.view_projection;
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
VertexOut outp;
outp.color = inp.i_color;
outp.border_radius = inp.i_border_radius;
outp.size = size;
outp.point = (inp.position - 0.5) * size;
outp.uv = inp.position;
outp.position = mvp * float4(vertices[vid], 0.0, 1.0);
outp.position.z = 1.0;
return outp;
}
float sd_rounded_box(float2 p, float2 size, float4 corner_radii) {
float2 rs = 0.0 < p.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < p.x ? rs.y : rs.x;
float2 corner_to_point = abs(p) - 0.5 * size;
float2 q = corner_to_point + radius;
float l = length(max(q, float2(0.0, 0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
float antialias(float d) {
float aa = pow(fwidth(d), 1.0 / 2.2);
return 1.0 - smoothstep(-aa, 0.0, d);
}
fragment float4 PS(
VertexOut inp [[stage_in]]
) {
float4 result = inp.color;
float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));
if (radius > 0.0) {
float external_distance = sd_rounded_box(inp.point, inp.size, inp.border_radius);
float smoothed_alpha = antialias(external_distance);
result = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
}
if (result.a < 0.05) discard_fragment();
return result;
})";

static const char METAL_NINEPATCH[3940] = R"(#include <metal_stdlib>
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
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
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
return map(coord, 0.0, out_margin.x, 0.0, source_margin.x);
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
const float2 horizontal_margin = float2(inp.margin.xy);
const float2 vertical_margin = float2(inp.margin.zw);
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

static const char METAL_SHAPE[6326] = R"(#include <metal_stdlib>
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
float2 p;
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
float4x4 transform = float4x4(
float4(1.0, 0.0, 0.0, 0.0),
float4(0.0, 1.0, 0.0, 0.0),
float4(0.0, 0.0, 1.0, 0.0),
float4(inp.i_position.x, inp.i_position.y, 0.0, 1.0)
);
const float2 offset = -inp.i_offset * inp.i_size;
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
transform[0] = transform[0] * inp.i_size[0];
transform[1] = transform[1] * inp.i_size[1];
const bool is_ui = (inp.i_flags & FLAG_UI) == FLAG_UI;
const float4x4 mvp = (is_ui ? constants.screen_projection : constants.view_projection) * transform;
const float2 position = inp.position;
VertexOut outp;
outp.position = mvp * float4(position, 0.0, 1.0);
outp.position.z = inp.i_position.z;
outp.uv = position;
outp.p = (position - 0.5) * inp.i_size;
outp.size = inp.i_size;
outp.color = inp.i_color;
outp.border_color = inp.i_border_color;
outp.border_thickness = inp.i_border_thickness;
outp.border_radius = inp.i_border_radius;
outp.shape = inp.i_shape;
return outp;
}
float sdf_circle(float2 p, float radius) {
return length(p) - radius;
}
constant constexpr float PI = 3.1415926535;
constant constexpr float TAU = 6.283185307179586;
float mod(float x, float y) {
return x - y * floor(x/y);
}
float arc_sdf(float2 p, float a0, float a1, float r )
{
float a = mod(atan2(p.y, p.x), TAU);
float ap = a - a0;
if (ap < 0.)
ap += TAU;
float a1p = a1 - a0;
if (a1p < 0.)
a1p += TAU;
if (ap >= a1p) {
float2 q0 = float2(r * cos(a0), r * sin(a0));
float2 q1 = float2(r * cos(a1), r * sin(a1));
return min(length(p - q0), length(p - q1));
}
return abs(length(p) - r);
}
float sd_rounded_box(float2 p, float2 size, float4 corner_radii) {
float2 rs = 0.0 < p.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < p.x ? rs.y : rs.x;
float2 corner_to_point = abs(p) - 0.5 * size;
float2 q = corner_to_point + radius;
float l = length(max(q, float2(0.0, 0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
float sd_inset_rounded_box(float2 p, float2 size, float4 radius, float4 inset) {
float2 inner_size = size - inset.xy - inset.zw;
float2 inner_center = inset.xy + 0.5 * inner_size - 0.5 * size;
float2 inner_point = p - inner_center;
float4 r = radius;
r.x = r.x - max(inset.x, inset.y);
r.y = r.y - max(inset.z, inset.y);
r.z = r.z - max(inset.z, inset.w);
r.w = r.w - max(inset.x, inset.w);
float2 half_size = inner_size * 0.5;
float min_size = min(half_size.x, half_size.y);
r = min(max(r, float4(0.0, 0.0, 0.0, 0.0)), float4(min_size, min_size, min_size, min_size));
return sd_rounded_box(inner_point, inner_size, r);
}
float antialias(float d) {
float afwidth = pow(fwidth(d), 1.0 / 2.2);
return 1.0 - smoothstep(0.0, afwidth, d);
}
float antialias_circle(float d) {
float afwidth = fwidth(d) * 1.3;
return 1.0 - smoothstep(0.0, afwidth, d);
}
fragment float4 PS(
VertexOut inp [[stage_in]],
texture2d<float> texture [[texture(3)]],
sampler texture_sampler [[sampler(4)]]
) {
float4 result = inp.color;
if (inp.shape == SHAPE_CIRCLE) {
float radius = 0.5 - 1.0 / inp.size.x;
float external_distance = sdf_circle(inp.uv - 0.5, radius);
float alpha = antialias_circle(external_distance);
if (inp.border_thickness > 0.0) {
float internal_distance = sdf_circle(inp.uv - 0.5, radius - inp.border_thickness / inp.size.x);
float border_distance = max(external_distance, -internal_distance);
float border_alpha = antialias_circle(border_distance);
float4 color = float4(inp.color.rgb, min(inp.color.a, alpha));
float4 color_with_border = mix(color, inp.border_color, min(inp.border_color.a, min(border_alpha, alpha)));
result = internal_distance > 0.0 && border_alpha < 1.0
? mix(float4(0.0, 0.0, 0.0, 0.0), inp.border_color, border_alpha)
: color_with_border;
} else {
result = float4(inp.color.rgb, min(inp.color.a, alpha));
}
} else if (inp.shape == SHAPE_ARC) {
float start_angle = inp.border_radius.x;
float end_angle = inp.border_radius.y;
float thickness = 0.5 - inp.border_thickness / inp.size.y;
float2 p = ((float2(inp.uv.x, 1.0 - inp.uv.y) * 2.0 - 1.0) * inp.size) / inp.size.y;
float d = arc_sdf(p, start_angle, end_angle, 1.0 - thickness);
float aa = length(float2(dfdx(d), dfdy(d)));
float alpha = 1.0 - smoothstep(thickness - aa, thickness, d);
result = float4(inp.color.rgb, min(inp.color.a, alpha));
}
float radius = max(inp.border_radius.x, max(inp.border_radius.y, max(inp.border_radius.z, inp.border_radius.w)));
if (radius > 0.0 || inp.border_thickness > 0.0) {
float external_distance = sd_rounded_box(inp.p, inp.size, inp.border_radius);
float internal_distance = sd_inset_rounded_box(inp.p, inp.size, inp.border_radius, float4(inp.border_thickness, inp.border_thickness, inp.border_thickness, inp.border_thickness));
float border_distance = max(external_distance, -internal_distance);
float border_alpha = antialias(border_distance);
float smoothed_alpha = antialias(external_distance);
float4 quad_color = float4(inp.color.rgb, min(inp.color.a, smoothed_alpha));
float4 quad_color_with_border = mix(quad_color, inp.border_color, min(inp.border_color.a, min(border_alpha, smoothed_alpha)));
result = internal_distance > 0.0 && border_alpha < 1.0
? float4(inp.border_color.rgb, border_alpha)
: quad_color_with_border;
}
if (result.a < 0.01) discard_fragment();
return result;
})";

static const char METAL_SPRITE[4352] = R"(#include <metal_stdlib>
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
constant constexpr int FLAG_UI = 1 << 0;
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
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
transform[0] = transform[0] * inp.i_size[0];
transform[1] = transform[1] * inp.i_size[1];
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
if (color.a < 0.025) discard_fragment();
return color;
}
)";

static const char GL_FONT_FRAG[408] = R"(#version 330 core
layout(location = 0) out vec4 frag_color;
in vec2 v_uv;
flat in vec3 v_color;
uniform sampler2D u_texture;
const float GLYPH_CENTER = 0.5;
void main() {
float dist = texture(u_texture, v_uv).r;
float width = fwidth(dist);
float alpha = smoothstep(GLYPH_CENTER - width, GLYPH_CENTER + width, abs(dist));
vec4 color = vec4(v_color, alpha);
if (color.a <= 0.05) discard;
frag_color = color;
})";

static const char GL_FONT_VERT[934] = R"(#version 330 core
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

static const char GL_LINE_FRAG[1570] = R"(#version 330 core
flat in vec4 v_color;
flat in vec4 v_border_radius;
in vec2 v_point;
in vec2 v_uv;
flat in vec2 v_size;
float sd_rounded_box(vec2 point, vec2 size, vec4 corner_radii) {
vec2 rs = 0.0 < point.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < point.x ? rs.y : rs.x;
vec2 corner_to_point = abs(point) - 0.5 * size;
vec2 q = corner_to_point + radius;
float l = length(max(q, vec2(0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
float sd_inset_rounded_box(vec2 point, vec2 size, vec4 radius, vec4 inset) {
vec2 inner_size = size - inset.xy - inset.zw;
vec2 inner_center = inset.xy + 0.5 * inner_size - 0.5 * size;
vec2 inner_point = point - inner_center;
vec4 r = radius;
r.x = r.x - max(inset.x, inset.y);
r.y = r.y - max(inset.z, inset.y);
r.z = r.z - max(inset.z, inset.w); 
r.w = r.w - max(inset.x, inset.w);
vec2 half_size = inner_size * 0.5;
float min_size = min(half_size.x, half_size.y);
r = min(max(r, vec4(0.0)), vec4(min_size));
return sd_rounded_box(inner_point, inner_size, r);
}
float antialias(float d) {
float aa = pow(fwidth(d), 1.0 / 2.2);
return 1.0 - smoothstep(-aa, 0.0, d);
}
out vec4 frag_color;
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
})";

static const char GL_LINE_VERT[1283] = R"(#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 i_start;
layout(location = 2) in vec2 i_end;
layout(location = 3) in vec4 i_color;
layout(location = 4) in vec4 i_border_radius;
layout(location = 5) in float i_thickness;
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
flat out vec4 v_color;
flat out vec4 v_border_radius;
out vec2 v_point;
out vec2 v_uv;
flat out vec2 v_size;
const uint FLAG_UI = 1u << 0u;
void main() {
bool is_ui = (i_flags & FLAG_UI) == FLAG_UI;
mat4 mvp = is_ui ? global_ubo.screen_projection : global_ubo.view_projection;
vec2 d = i_end - i_start;
float len = length(d);
vec2 perp = (vec2(d.y, -d.x) / len) * i_thickness * 0.5;
vec2 vertices[4] = vec2[4](
vec2(i_start - perp),
vec2(i_start + perp),
vec2(i_end - perp),
vec2(i_end + perp)
);
vec2 size = vec2(len, i_thickness);
v_color = i_color;
v_border_radius = i_border_radius;
v_point = (a_position - 0.5) * size;
v_uv = a_position;
v_size = size;
gl_Position = mvp * vec4(vertices[gl_VertexID], 0.0, 1.0);
gl_Position.z = 1.0;
})";

static const char GL_NINEPATCH_FRAG[1116] = R"(#version 330 core
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

static const char GL_NINEPATCH_VERT[2498] = R"(#version 330 core
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
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
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

static const char GL_SHAPE_FRAG[4194] = R"(#version 330 core
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
if (ap >= a1p) {
vec2 q0 = vec2(r * cos(a0), r * sin(a0));
vec2 q1 = vec2(r * cos(a1), r * sin(a1));
return min(length(p - q0), length(p - q1));
}
return abs(length(p) - r);
}
float sd_rounded_box(vec2 point, vec2 size, vec4 corner_radii) {
vec2 rs = 0.0 < point.y ? corner_radii.zw : corner_radii.xy;
float radius = 0.0 < point.x ? rs.y : rs.x;
vec2 corner_to_point = abs(point) - 0.5 * size;
vec2 q = corner_to_point + radius;
float l = length(max(q, vec2(0.0)));
float m = min(max(q.x, q.y), 0.0);
return l + m - radius;
}
float sd_inset_rounded_box(vec2 point, vec2 size, vec4 radius, vec4 inset) {
vec2 inner_size = size - inset.xy - inset.zw;
vec2 inner_center = inset.xy + 0.5 * inner_size - 0.5 * size;
vec2 inner_point = point - inner_center;
vec4 r = radius;
r.x = r.x - max(inset.x, inset.y);
r.y = r.y - max(inset.z, inset.y);
r.z = r.z - max(inset.z, inset.w);
r.w = r.w - max(inset.x, inset.w);
vec2 half_size = inner_size * 0.5;
float min_size = min(half_size.x, half_size.y);
r = min(max(r, vec4(0.0)), vec4(min_size));
return sd_rounded_box(inner_point, inner_size, r);
}
const uint SHAPE_CIRCLE = 1u;
const uint SHAPE_ARC = 2u;
float antialias(float d) {
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
float external_distance = sd_rounded_box(v_point, v_size, v_border_radius);
float internal_distance = sd_inset_rounded_box(v_point, v_size, v_border_radius, vec4(v_border_thickness));
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
})";

static const char GL_SHAPE_VERT[1709] = R"(#version 330 core
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
out vec2 v_point;
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
mat4 transform = mat4(
vec4(1.0, 0.0, 0.0, 0.0),
vec4(0.0, 1.0, 0.0, 0.0),
vec4(0.0, 0.0, 1.0, 0.0),
vec4(i_position.x, i_position.y, 0.0, 1.0)
);
vec2 offset = -i_offset * i_size;
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
transform[0] = transform[0] * i_size[0];
transform[1] = transform[1] * i_size[1];
bool is_ui = (i_flags & IS_UI_FLAG) == IS_UI_FLAG;
mat4 mvp = (is_ui ? global_ubo.screen_projection : global_ubo.view_projection) * transform;
v_uv = a_position;
v_point = (a_position - 0.5) * i_size;
v_color = i_color;
v_size = i_size;
v_border_color = i_border_color;
v_border_thickness = i_border_thickness;
v_border_radius = i_border_radius;
v_shape = i_shape;
gl_Position = mvp * vec4(a_position, 0, 1);
gl_Position.z = i_position.z;
})";

static const char GL_SPRITE_FRAG[1103] = R"(#version 330 core
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
if (color.a < 0.025) discard;
frag_color = color;
}
)";

static const char GL_SPRITE_VERT[2454] = R"(#version 330 core
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
transform[3] = transform[0] * offset[0] + transform[1] * offset[1] + transform[2] * 0.0 + transform[3];
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

static const unsigned char VULKAN_FONT_FRAG[1516] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 63, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 8, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 21, 0, 0, 0, 44, 0, 0, 0, 61, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 100, 105, 115, 116, 0, 0, 0, 0, 5, 0, 5, 0, 11, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 15, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 21, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 28, 0, 0, 0, 119, 105, 100, 116, 104, 0, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 4, 0, 41, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 4, 0, 44, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 61, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 21, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 44, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 44, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 61, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 25, 0, 9, 0, 9, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 13, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 17, 0, 0, 0, 9, 0, 0, 0, 23, 0, 4, 0, 19, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 20, 0, 0, 0, 1, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 20, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 23, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 25, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 25, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 40, 0, 0, 0, 7, 0, 0, 0, 23, 0, 0, 0, 23, 0, 4, 0, 42, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 43, 0, 0, 0, 1, 0, 0, 0, 42, 0, 0, 0, 59, 0, 4, 0, 43, 0, 0, 0, 44, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 25, 0, 0, 0, 51, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 205, 204, 76, 61, 20, 0, 2, 0, 55, 0, 0, 0, 32, 0, 4, 0, 60, 0, 0, 0, 3, 0, 0, 0, 23, 0, 0, 0, 59, 0, 4, 0, 60, 0, 0, 0, 61, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 28, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 31, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 40, 0, 0, 0, 41, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 13, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 86, 0, 5, 0, 17, 0, 0, 0, 18, 0, 0, 0, 12, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 22, 0, 0, 0, 21, 0, 0, 0, 87, 0, 5, 0, 23, 0, 0, 0, 24, 0, 0, 0, 18, 0, 0, 0, 22, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 27, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 27, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 29, 0, 0, 0, 8, 0, 0, 0, 209, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 62, 0, 3, 0, 28, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 33, 0, 0, 0, 28, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 32, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 0, 0, 0, 28, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 32, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 37, 0, 0, 0, 8, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 38, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 37, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 39, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 31, 0, 0, 0, 39, 0, 0, 0, 61, 0, 4, 0, 42, 0, 0, 0, 45, 0, 0, 0, 44, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 46, 0, 0, 0, 31, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 47, 0, 0, 0, 45, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 48, 0, 0, 0, 45, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 49, 0, 0, 0, 45, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 23, 0, 0, 0, 50, 0, 0, 0, 47, 0, 0, 0, 48, 0, 0, 0, 49, 0, 0, 0, 46, 0, 0, 0, 62, 0, 3, 0, 41, 0, 0, 0, 50, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 52, 0, 0, 0, 41, 0, 0, 0, 51, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 188, 0, 5, 0, 55, 0, 0, 0, 56, 0, 0, 0, 53, 0, 0, 0, 54, 0, 0, 0, 247, 0, 3, 0, 58, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 56, 0, 0, 0, 57, 0, 0, 0, 58, 0, 0, 0, 248, 0, 2, 0, 57, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 58, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 62, 0, 0, 0, 41, 0, 0, 0, 62, 0, 3, 0, 61, 0, 0, 0, 62, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_FONT_VERT[3032] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 85, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 15, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 43, 0, 0, 0, 47, 0, 0, 0, 49, 0, 0, 0, 54, 0, 0, 0, 57, 0, 0, 0, 62, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 71, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 20, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 26, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 26, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 26, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 26, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 26, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 26, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 28, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 5, 0, 40, 0, 0, 0, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 0, 0, 5, 0, 5, 0, 43, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 5, 0, 47, 0, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 49, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 3, 0, 53, 0, 0, 0, 117, 118, 0, 0, 5, 0, 4, 0, 54, 0, 0, 0, 105, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 57, 0, 0, 0, 105, 95, 116, 101, 120, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 62, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 65, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 66, 0, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 69, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 69, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 69, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 69, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 69, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 71, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 26, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 43, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 47, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 49, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 54, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 57, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 62, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 65, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 65, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 66, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 69, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 69, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 6, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 21, 0, 4, 0, 9, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 22, 0, 3, 0, 16, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 17, 0, 0, 0, 16, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 18, 0, 0, 0, 17, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 19, 0, 0, 0, 7, 0, 0, 0, 18, 0, 0, 0, 23, 0, 4, 0, 25, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 30, 0, 10, 0, 26, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 25, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 21, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 31, 0, 0, 0, 2, 0, 0, 0, 18, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 39, 0, 0, 0, 7, 0, 0, 0, 25, 0, 0, 0, 23, 0, 4, 0, 41, 0, 0, 0, 16, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 42, 0, 0, 0, 1, 0, 0, 0, 41, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 43, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 46, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 47, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 49, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 54, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 46, 0, 0, 0, 57, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 61, 0, 0, 0, 3, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 61, 0, 0, 0, 62, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 64, 0, 0, 0, 3, 0, 0, 0, 41, 0, 0, 0, 59, 0, 4, 0, 64, 0, 0, 0, 65, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 42, 0, 0, 0, 66, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 68, 0, 0, 0, 16, 0, 0, 0, 13, 0, 0, 0, 30, 0, 6, 0, 69, 0, 0, 0, 17, 0, 0, 0, 16, 0, 0, 0, 68, 0, 0, 0, 68, 0, 0, 0, 32, 0, 4, 0, 70, 0, 0, 0, 3, 0, 0, 0, 69, 0, 0, 0, 59, 0, 4, 0, 70, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 74, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 75, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 80, 0, 0, 0, 3, 0, 0, 0, 17, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 83, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 53, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 199, 0, 5, 0, 9, 0, 0, 0, 14, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 170, 0, 5, 0, 6, 0, 0, 0, 15, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 21, 0, 0, 0, 8, 0, 0, 0, 247, 0, 3, 0, 24, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 21, 0, 0, 0, 23, 0, 0, 0, 34, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 28, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 33, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 33, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 34, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 36, 0, 0, 0, 28, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 37, 0, 0, 0, 36, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 37, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 24, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 38, 0, 0, 0, 22, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 38, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 79, 0, 7, 0, 25, 0, 0, 0, 45, 0, 0, 0, 44, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 52, 0, 0, 0, 45, 0, 0, 0, 51, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 52, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 56, 0, 0, 0, 47, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 59, 0, 0, 0, 56, 0, 0, 0, 58, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 60, 0, 0, 0, 55, 0, 0, 0, 59, 0, 0, 0, 62, 0, 3, 0, 53, 0, 0, 0, 60, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 63, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 62, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 41, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 72, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 73, 0, 0, 0, 40, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 76, 0, 0, 0, 73, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 77, 0, 0, 0, 73, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 17, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 74, 0, 0, 0, 75, 0, 0, 0, 145, 0, 5, 0, 17, 0, 0, 0, 79, 0, 0, 0, 72, 0, 0, 0, 78, 0, 0, 0, 65, 0, 5, 0, 80, 0, 0, 0, 81, 0, 0, 0, 71, 0, 0, 0, 30, 0, 0, 0, 62, 0, 3, 0, 81, 0, 0, 0, 79, 0, 0, 0, 65, 0, 6, 0, 83, 0, 0, 0, 84, 0, 0, 0, 71, 0, 0, 0, 30, 0, 0, 0, 82, 0, 0, 0, 62, 0, 3, 0, 84, 0, 0, 0, 75, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_LINE_FRAG[3800] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 158, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 11, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 99, 0, 0, 0, 102, 0, 0, 0, 123, 0, 0, 0, 124, 0, 0, 0, 155, 0, 0, 0, 157, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 9, 0, 15, 0, 0, 0, 115, 100, 95, 114, 111, 117, 110, 100, 101, 100, 95, 98, 111, 120, 40, 118, 102, 50, 59, 118, 102, 50, 59, 118, 102, 52, 59, 0, 5, 0, 4, 0, 12, 0, 0, 0, 112, 111, 105, 110, 116, 0, 0, 0, 5, 0, 4, 0, 13, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 6, 0, 14, 0, 0, 0, 99, 111, 114, 110, 101, 114, 95, 114, 97, 100, 105, 105, 0, 0, 0, 0, 5, 0, 6, 0, 20, 0, 0, 0, 97, 110, 116, 105, 97, 108, 105, 97, 115, 40, 102, 49, 59, 0, 0, 0, 5, 0, 3, 0, 19, 0, 0, 0, 100, 0, 0, 0, 5, 0, 3, 0, 22, 0, 0, 0, 114, 115, 0, 0, 5, 0, 4, 0, 39, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 6, 0, 53, 0, 0, 0, 99, 111, 114, 110, 101, 114, 95, 116, 111, 95, 112, 111, 105, 110, 116, 0, 5, 0, 3, 0, 60, 0, 0, 0, 113, 0, 0, 0, 5, 0, 3, 0, 65, 0, 0, 0, 108, 0, 0, 0, 5, 0, 3, 0, 70, 0, 0, 0, 109, 0, 0, 0, 5, 0, 3, 0, 84, 0, 0, 0, 97, 97, 0, 0, 5, 0, 4, 0, 97, 0, 0, 0, 114, 101, 115, 117, 108, 116, 0, 0, 5, 0, 4, 0, 99, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 101, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 6, 0, 102, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 7, 0, 121, 0, 0, 0, 101, 120, 116, 101, 114, 110, 97, 108, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 0, 0, 5, 0, 4, 0, 123, 0, 0, 0, 118, 95, 112, 111, 105, 110, 116, 0, 5, 0, 4, 0, 124, 0, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 125, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 127, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 129, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 6, 0, 132, 0, 0, 0, 115, 109, 111, 111, 116, 104, 101, 100, 95, 97, 108, 112, 104, 97, 0, 0, 5, 0, 4, 0, 133, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 5, 0, 155, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 4, 0, 157, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 71, 0, 3, 0, 99, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 99, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 102, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 102, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 123, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 124, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 124, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 155, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 157, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 33, 0, 6, 0, 11, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 10, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 33, 0, 4, 0, 18, 0, 0, 0, 6, 0, 0, 0, 17, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 21, 0, 4, 0, 24, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 24, 0, 0, 0, 25, 0, 0, 0, 1, 0, 0, 0, 20, 0, 2, 0, 28, 0, 0, 0, 43, 0, 4, 0, 24, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 63, 44, 0, 5, 0, 7, 0, 0, 0, 67, 0, 0, 0, 23, 0, 0, 0, 23, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 87, 0, 0, 0, 47, 186, 232, 62, 43, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 0, 0, 128, 63, 32, 0, 4, 0, 98, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 98, 0, 0, 0, 99, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 98, 0, 0, 0, 102, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 103, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 24, 0, 0, 0, 108, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 24, 0, 0, 0, 111, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 122, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 123, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 136, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 149, 0, 0, 0, 205, 204, 76, 61, 32, 0, 4, 0, 154, 0, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 154, 0, 0, 0, 155, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 157, 0, 0, 0, 1, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 97, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 101, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 121, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 125, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 127, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 129, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 132, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 133, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 100, 0, 0, 0, 99, 0, 0, 0, 62, 0, 3, 0, 97, 0, 0, 0, 100, 0, 0, 0, 65, 0, 5, 0, 103, 0, 0, 0, 104, 0, 0, 0, 102, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 105, 0, 0, 0, 104, 0, 0, 0, 65, 0, 5, 0, 103, 0, 0, 0, 106, 0, 0, 0, 102, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 107, 0, 0, 0, 106, 0, 0, 0, 65, 0, 5, 0, 103, 0, 0, 0, 109, 0, 0, 0, 102, 0, 0, 0, 108, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 65, 0, 5, 0, 103, 0, 0, 0, 112, 0, 0, 0, 102, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 114, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 110, 0, 0, 0, 113, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 107, 0, 0, 0, 114, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 116, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 105, 0, 0, 0, 115, 0, 0, 0, 62, 0, 3, 0, 101, 0, 0, 0, 116, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 117, 0, 0, 0, 101, 0, 0, 0, 186, 0, 5, 0, 28, 0, 0, 0, 118, 0, 0, 0, 117, 0, 0, 0, 23, 0, 0, 0, 247, 0, 3, 0, 120, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 118, 0, 0, 0, 119, 0, 0, 0, 120, 0, 0, 0, 248, 0, 2, 0, 119, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 126, 0, 0, 0, 123, 0, 0, 0, 62, 0, 3, 0, 125, 0, 0, 0, 126, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 128, 0, 0, 0, 124, 0, 0, 0, 62, 0, 3, 0, 127, 0, 0, 0, 128, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 130, 0, 0, 0, 102, 0, 0, 0, 62, 0, 3, 0, 129, 0, 0, 0, 130, 0, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 131, 0, 0, 0, 15, 0, 0, 0, 125, 0, 0, 0, 127, 0, 0, 0, 129, 0, 0, 0, 62, 0, 3, 0, 121, 0, 0, 0, 131, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 134, 0, 0, 0, 121, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 134, 0, 0, 0, 57, 0, 5, 0, 6, 0, 0, 0, 135, 0, 0, 0, 20, 0, 0, 0, 133, 0, 0, 0, 62, 0, 3, 0, 132, 0, 0, 0, 135, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 137, 0, 0, 0, 99, 0, 0, 0, 79, 0, 8, 0, 136, 0, 0, 0, 138, 0, 0, 0, 137, 0, 0, 0, 137, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 103, 0, 0, 0, 139, 0, 0, 0, 99, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 140, 0, 0, 0, 139, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 141, 0, 0, 0, 132, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 142, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 140, 0, 0, 0, 141, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 143, 0, 0, 0, 138, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 144, 0, 0, 0, 138, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 145, 0, 0, 0, 138, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 146, 0, 0, 0, 143, 0, 0, 0, 144, 0, 0, 0, 145, 0, 0, 0, 142, 0, 0, 0, 62, 0, 3, 0, 97, 0, 0, 0, 146, 0, 0, 0, 249, 0, 2, 0, 120, 0, 0, 0, 248, 0, 2, 0, 120, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 147, 0, 0, 0, 97, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 188, 0, 5, 0, 28, 0, 0, 0, 150, 0, 0, 0, 148, 0, 0, 0, 149, 0, 0, 0, 247, 0, 3, 0, 152, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 150, 0, 0, 0, 151, 0, 0, 0, 152, 0, 0, 0, 248, 0, 2, 0, 151, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 152, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 156, 0, 0, 0, 97, 0, 0, 0, 62, 0, 3, 0, 155, 0, 0, 0, 156, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 12, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 13, 0, 0, 0, 55, 0, 3, 0, 10, 0, 0, 0, 14, 0, 0, 0, 248, 0, 2, 0, 16, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 39, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 44, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 53, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 60, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 70, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 26, 0, 0, 0, 12, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 27, 0, 0, 0, 26, 0, 0, 0, 184, 0, 5, 0, 28, 0, 0, 0, 29, 0, 0, 0, 23, 0, 0, 0, 27, 0, 0, 0, 247, 0, 3, 0, 32, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 29, 0, 0, 0, 31, 0, 0, 0, 35, 0, 0, 0, 248, 0, 2, 0, 31, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 33, 0, 0, 0, 14, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 34, 0, 0, 0, 33, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 62, 0, 3, 0, 30, 0, 0, 0, 34, 0, 0, 0, 249, 0, 2, 0, 32, 0, 0, 0, 248, 0, 2, 0, 35, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 36, 0, 0, 0, 14, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 37, 0, 0, 0, 36, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 62, 0, 3, 0, 30, 0, 0, 0, 37, 0, 0, 0, 249, 0, 2, 0, 32, 0, 0, 0, 248, 0, 2, 0, 32, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 38, 0, 0, 0, 30, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 38, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 41, 0, 0, 0, 12, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 184, 0, 5, 0, 28, 0, 0, 0, 43, 0, 0, 0, 23, 0, 0, 0, 42, 0, 0, 0, 247, 0, 3, 0, 46, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 43, 0, 0, 0, 45, 0, 0, 0, 49, 0, 0, 0, 248, 0, 2, 0, 45, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 47, 0, 0, 0, 22, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 62, 0, 3, 0, 44, 0, 0, 0, 48, 0, 0, 0, 249, 0, 2, 0, 46, 0, 0, 0, 248, 0, 2, 0, 49, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 50, 0, 0, 0, 22, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 51, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 44, 0, 0, 0, 51, 0, 0, 0, 249, 0, 2, 0, 46, 0, 0, 0, 248, 0, 2, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 52, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 39, 0, 0, 0, 52, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 54, 0, 0, 0, 12, 0, 0, 0, 12, 0, 6, 0, 7, 0, 0, 0, 55, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 57, 0, 0, 0, 13, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 59, 0, 0, 0, 55, 0, 0, 0, 58, 0, 0, 0, 62, 0, 3, 0, 53, 0, 0, 0, 59, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 61, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 62, 0, 0, 0, 39, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 62, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 60, 0, 0, 0, 64, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 66, 0, 0, 0, 60, 0, 0, 0, 12, 0, 7, 0, 7, 0, 0, 0, 68, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 66, 0, 0, 0, 67, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 69, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 68, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 69, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 71, 0, 0, 0, 60, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 72, 0, 0, 0, 71, 0, 0, 0, 65, 0, 5, 0, 17, 0, 0, 0, 73, 0, 0, 0, 60, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 73, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 72, 0, 0, 0, 74, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 76, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 75, 0, 0, 0, 23, 0, 0, 0, 62, 0, 3, 0, 70, 0, 0, 0, 76, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 65, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 78, 0, 0, 0, 70, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 79, 0, 0, 0, 77, 0, 0, 0, 78, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 80, 0, 0, 0, 39, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 81, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 254, 0, 2, 0, 81, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 55, 0, 3, 0, 17, 0, 0, 0, 19, 0, 0, 0, 248, 0, 2, 0, 21, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 84, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 19, 0, 0, 0, 209, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 85, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 88, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 86, 0, 0, 0, 87, 0, 0, 0, 62, 0, 3, 0, 84, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 90, 0, 0, 0, 84, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 90, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 19, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 93, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 91, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 89, 0, 0, 0, 93, 0, 0, 0, 254, 0, 2, 0, 94, 0, 0, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_LINE_VERT[4100] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 129, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 19, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 42, 0, 0, 0, 44, 0, 0, 0, 63, 0, 0, 0, 90, 0, 0, 0, 92, 0, 0, 0, 94, 0, 0, 0, 95, 0, 0, 0, 98, 0, 0, 0, 99, 0, 0, 0, 105, 0, 0, 0, 107, 0, 0, 0, 112, 0, 0, 0, 115, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 8, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 20, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 26, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 26, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 10, 0, 26, 0, 0, 0, 2, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 0, 6, 0, 8, 0, 26, 0, 0, 0, 3, 0, 0, 0, 110, 111, 110, 115, 99, 97, 108, 101, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 26, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 26, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 26, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 28, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 3, 0, 40, 0, 0, 0, 100, 0, 0, 0, 5, 0, 4, 0, 42, 0, 0, 0, 105, 95, 101, 110, 100, 0, 0, 0, 5, 0, 4, 0, 44, 0, 0, 0, 105, 95, 115, 116, 97, 114, 116, 0, 5, 0, 3, 0, 48, 0, 0, 0, 108, 101, 110, 0, 5, 0, 4, 0, 51, 0, 0, 0, 112, 101, 114, 112, 0, 0, 0, 0, 5, 0, 5, 0, 63, 0, 0, 0, 105, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 5, 0, 71, 0, 0, 0, 118, 101, 114, 116, 105, 99, 101, 115, 0, 0, 0, 0, 5, 0, 4, 0, 85, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 4, 0, 90, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 92, 0, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 94, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 6, 0, 95, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 4, 0, 98, 0, 0, 0, 118, 95, 112, 111, 105, 110, 116, 0, 5, 0, 5, 0, 99, 0, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 105, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 107, 0, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 110, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 110, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 110, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 110, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 110, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 112, 0, 0, 0, 0, 0, 0, 0, 5, 0, 6, 0, 115, 0, 0, 0, 103, 108, 95, 86, 101, 114, 116, 101, 120, 73, 110, 100, 101, 120, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 26, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 4, 0, 26, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 26, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 42, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 44, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 63, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 90, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 90, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 92, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 94, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 94, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 95, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 98, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 99, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 105, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 107, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 107, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 110, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 115, 0, 0, 0, 11, 0, 0, 0, 42, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 6, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 21, 0, 4, 0, 9, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 22, 0, 3, 0, 16, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 17, 0, 0, 0, 16, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 18, 0, 0, 0, 17, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 19, 0, 0, 0, 7, 0, 0, 0, 18, 0, 0, 0, 23, 0, 4, 0, 25, 0, 0, 0, 16, 0, 0, 0, 2, 0, 0, 0, 30, 0, 10, 0, 26, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 18, 0, 0, 0, 25, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 2, 0, 0, 0, 26, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 21, 0, 4, 0, 29, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 31, 0, 0, 0, 2, 0, 0, 0, 18, 0, 0, 0, 43, 0, 4, 0, 29, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 39, 0, 0, 0, 7, 0, 0, 0, 25, 0, 0, 0, 32, 0, 4, 0, 41, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 41, 0, 0, 0, 42, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 41, 0, 0, 0, 44, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 47, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 43, 0, 4, 0, 9, 0, 0, 0, 54, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 62, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 62, 0, 0, 0, 63, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 63, 43, 0, 4, 0, 9, 0, 0, 0, 68, 0, 0, 0, 4, 0, 0, 0, 28, 0, 4, 0, 69, 0, 0, 0, 25, 0, 0, 0, 68, 0, 0, 0, 32, 0, 4, 0, 70, 0, 0, 0, 7, 0, 0, 0, 69, 0, 0, 0, 32, 0, 4, 0, 89, 0, 0, 0, 3, 0, 0, 0, 17, 0, 0, 0, 59, 0, 4, 0, 89, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 91, 0, 0, 0, 1, 0, 0, 0, 17, 0, 0, 0, 59, 0, 4, 0, 91, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 89, 0, 0, 0, 94, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 91, 0, 0, 0, 95, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 97, 0, 0, 0, 3, 0, 0, 0, 25, 0, 0, 0, 59, 0, 4, 0, 97, 0, 0, 0, 98, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 41, 0, 0, 0, 99, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 97, 0, 0, 0, 105, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 97, 0, 0, 0, 107, 0, 0, 0, 3, 0, 0, 0, 28, 0, 4, 0, 109, 0, 0, 0, 16, 0, 0, 0, 13, 0, 0, 0, 30, 0, 6, 0, 110, 0, 0, 0, 17, 0, 0, 0, 16, 0, 0, 0, 109, 0, 0, 0, 109, 0, 0, 0, 32, 0, 4, 0, 111, 0, 0, 0, 3, 0, 0, 0, 110, 0, 0, 0, 59, 0, 4, 0, 111, 0, 0, 0, 112, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 114, 0, 0, 0, 1, 0, 0, 0, 29, 0, 0, 0, 59, 0, 4, 0, 114, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 119, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 16, 0, 0, 0, 120, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 9, 0, 0, 0, 126, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 127, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 47, 0, 0, 0, 48, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 70, 0, 0, 0, 71, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 39, 0, 0, 0, 85, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 199, 0, 5, 0, 9, 0, 0, 0, 14, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 170, 0, 5, 0, 6, 0, 0, 0, 15, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 21, 0, 0, 0, 8, 0, 0, 0, 247, 0, 3, 0, 24, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 21, 0, 0, 0, 23, 0, 0, 0, 34, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 28, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 33, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 33, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 34, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 36, 0, 0, 0, 28, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 37, 0, 0, 0, 36, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 37, 0, 0, 0, 249, 0, 2, 0, 24, 0, 0, 0, 248, 0, 2, 0, 24, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 38, 0, 0, 0, 22, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 38, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 43, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 45, 0, 0, 0, 44, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 46, 0, 0, 0, 43, 0, 0, 0, 45, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 49, 0, 0, 0, 40, 0, 0, 0, 12, 0, 6, 0, 16, 0, 0, 0, 50, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 49, 0, 0, 0, 62, 0, 3, 0, 48, 0, 0, 0, 50, 0, 0, 0, 65, 0, 5, 0, 47, 0, 0, 0, 52, 0, 0, 0, 40, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 65, 0, 5, 0, 47, 0, 0, 0, 55, 0, 0, 0, 40, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 56, 0, 0, 0, 55, 0, 0, 0, 127, 0, 4, 0, 16, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 58, 0, 0, 0, 53, 0, 0, 0, 57, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 59, 0, 0, 0, 48, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 60, 0, 0, 0, 59, 0, 0, 0, 59, 0, 0, 0, 136, 0, 5, 0, 25, 0, 0, 0, 61, 0, 0, 0, 58, 0, 0, 0, 60, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 142, 0, 5, 0, 25, 0, 0, 0, 65, 0, 0, 0, 61, 0, 0, 0, 64, 0, 0, 0, 142, 0, 5, 0, 25, 0, 0, 0, 67, 0, 0, 0, 65, 0, 0, 0, 66, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 72, 0, 0, 0, 44, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 73, 0, 0, 0, 51, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 74, 0, 0, 0, 72, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 75, 0, 0, 0, 44, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 76, 0, 0, 0, 51, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 77, 0, 0, 0, 75, 0, 0, 0, 76, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 78, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 79, 0, 0, 0, 51, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 80, 0, 0, 0, 78, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 81, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 82, 0, 0, 0, 51, 0, 0, 0, 129, 0, 5, 0, 25, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 80, 0, 7, 0, 69, 0, 0, 0, 84, 0, 0, 0, 74, 0, 0, 0, 77, 0, 0, 0, 80, 0, 0, 0, 83, 0, 0, 0, 62, 0, 3, 0, 71, 0, 0, 0, 84, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 86, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 87, 0, 0, 0, 63, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 88, 0, 0, 0, 86, 0, 0, 0, 87, 0, 0, 0, 62, 0, 3, 0, 85, 0, 0, 0, 88, 0, 0, 0, 61, 0, 4, 0, 17, 0, 0, 0, 93, 0, 0, 0, 92, 0, 0, 0, 62, 0, 3, 0, 90, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 17, 0, 0, 0, 96, 0, 0, 0, 95, 0, 0, 0, 62, 0, 3, 0, 94, 0, 0, 0, 96, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 100, 0, 0, 0, 99, 0, 0, 0, 80, 0, 5, 0, 25, 0, 0, 0, 101, 0, 0, 0, 66, 0, 0, 0, 66, 0, 0, 0, 131, 0, 5, 0, 25, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 103, 0, 0, 0, 85, 0, 0, 0, 133, 0, 5, 0, 25, 0, 0, 0, 104, 0, 0, 0, 102, 0, 0, 0, 103, 0, 0, 0, 62, 0, 3, 0, 98, 0, 0, 0, 104, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 106, 0, 0, 0, 99, 0, 0, 0, 62, 0, 3, 0, 105, 0, 0, 0, 106, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 108, 0, 0, 0, 85, 0, 0, 0, 62, 0, 3, 0, 107, 0, 0, 0, 108, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 113, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 29, 0, 0, 0, 116, 0, 0, 0, 115, 0, 0, 0, 65, 0, 5, 0, 39, 0, 0, 0, 117, 0, 0, 0, 71, 0, 0, 0, 116, 0, 0, 0, 61, 0, 4, 0, 25, 0, 0, 0, 118, 0, 0, 0, 117, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 121, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 16, 0, 0, 0, 122, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 17, 0, 0, 0, 123, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 119, 0, 0, 0, 120, 0, 0, 0, 145, 0, 5, 0, 17, 0, 0, 0, 124, 0, 0, 0, 113, 0, 0, 0, 123, 0, 0, 0, 65, 0, 5, 0, 89, 0, 0, 0, 125, 0, 0, 0, 112, 0, 0, 0, 30, 0, 0, 0, 62, 0, 3, 0, 125, 0, 0, 0, 124, 0, 0, 0, 65, 0, 6, 0, 127, 0, 0, 0, 128, 0, 0, 0, 112, 0, 0, 0, 30, 0, 0, 0, 126, 0, 0, 0, 62, 0, 3, 0, 128, 0, 0, 0, 120, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_NINEPATCH_FRAG[4536] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 180, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 11, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 106, 0, 0, 0, 117, 0, 0, 0, 119, 0, 0, 0, 124, 0, 0, 0, 166, 0, 0, 0, 178, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 7, 0, 14, 0, 0, 0, 109, 97, 112, 40, 102, 49, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 0, 5, 0, 4, 0, 9, 0, 0, 0, 118, 97, 108, 117, 101, 0, 0, 0, 5, 0, 4, 0, 10, 0, 0, 0, 105, 110, 95, 109, 105, 110, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 105, 110, 95, 109, 97, 120, 0, 0, 5, 0, 4, 0, 12, 0, 0, 0, 111, 117, 116, 95, 109, 105, 110, 0, 5, 0, 4, 0, 13, 0, 0, 0, 111, 117, 116, 95, 109, 97, 120, 0, 5, 0, 9, 0, 22, 0, 0, 0, 112, 114, 111, 99, 101, 115, 115, 95, 97, 120, 105, 115, 40, 102, 49, 59, 118, 102, 50, 59, 118, 102, 50, 59, 0, 0, 0, 0, 5, 0, 4, 0, 19, 0, 0, 0, 99, 111, 111, 114, 100, 0, 0, 0, 5, 0, 6, 0, 20, 0, 0, 0, 115, 111, 117, 114, 99, 101, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 5, 0, 5, 0, 21, 0, 0, 0, 111, 117, 116, 95, 109, 97, 114, 103, 105, 110, 0, 0, 5, 0, 4, 0, 49, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 51, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 52, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 55, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 56, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 76, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 78, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 81, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 82, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 85, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 94, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 96, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 97, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 98, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 99, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 7, 0, 103, 0, 0, 0, 104, 111, 114, 105, 122, 111, 110, 116, 97, 108, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 5, 0, 5, 0, 106, 0, 0, 0, 118, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 111, 0, 0, 0, 118, 101, 114, 116, 105, 99, 97, 108, 95, 109, 97, 114, 103, 105, 110, 0, 5, 0, 4, 0, 115, 0, 0, 0, 110, 101, 119, 95, 117, 118, 0, 0, 5, 0, 4, 0, 117, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 6, 0, 119, 0, 0, 0, 118, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 124, 0, 0, 0, 118, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 4, 0, 128, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 132, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 133, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 143, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 146, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 147, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 152, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 5, 0, 155, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 159, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 166, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 178, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 3, 0, 106, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 106, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 117, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 119, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 119, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 124, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 124, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 155, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 155, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 159, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 159, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 166, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 166, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 178, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 33, 0, 8, 0, 8, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 23, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 33, 0, 6, 0, 18, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 17, 0, 0, 0, 17, 0, 0, 0, 21, 0, 4, 0, 40, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 20, 0, 2, 0, 44, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 62, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 40, 0, 0, 0, 63, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 104, 0, 0, 0, 40, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 105, 0, 0, 0, 1, 0, 0, 0, 104, 0, 0, 0, 59, 0, 4, 0, 105, 0, 0, 0, 106, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 107, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 116, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 117, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 116, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 129, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 150, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 151, 0, 0, 0, 7, 0, 0, 0, 150, 0, 0, 0, 25, 0, 9, 0, 153, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 154, 0, 0, 0, 0, 0, 0, 0, 153, 0, 0, 0, 59, 0, 4, 0, 154, 0, 0, 0, 155, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 157, 0, 0, 0, 32, 0, 4, 0, 158, 0, 0, 0, 0, 0, 0, 0, 157, 0, 0, 0, 59, 0, 4, 0, 158, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 161, 0, 0, 0, 153, 0, 0, 0, 32, 0, 4, 0, 165, 0, 0, 0, 1, 0, 0, 0, 150, 0, 0, 0, 59, 0, 4, 0, 165, 0, 0, 0, 166, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 40, 0, 0, 0, 169, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 172, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 177, 0, 0, 0, 3, 0, 0, 0, 150, 0, 0, 0, 59, 0, 4, 0, 177, 0, 0, 0, 178, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 103, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 111, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 115, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 128, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 132, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 133, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 143, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 146, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 147, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 151, 0, 0, 0, 152, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 104, 0, 0, 0, 108, 0, 0, 0, 106, 0, 0, 0, 79, 0, 7, 0, 107, 0, 0, 0, 109, 0, 0, 0, 108, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 112, 0, 4, 0, 16, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 62, 0, 3, 0, 103, 0, 0, 0, 110, 0, 0, 0, 61, 0, 4, 0, 104, 0, 0, 0, 112, 0, 0, 0, 106, 0, 0, 0, 79, 0, 7, 0, 107, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 112, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 112, 0, 4, 0, 16, 0, 0, 0, 114, 0, 0, 0, 113, 0, 0, 0, 62, 0, 3, 0, 111, 0, 0, 0, 114, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 118, 0, 0, 0, 103, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 120, 0, 0, 0, 119, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 122, 0, 0, 0, 118, 0, 0, 0, 121, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 123, 0, 0, 0, 103, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 125, 0, 0, 0, 124, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 126, 0, 0, 0, 125, 0, 0, 0, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 127, 0, 0, 0, 123, 0, 0, 0, 126, 0, 0, 0, 65, 0, 5, 0, 129, 0, 0, 0, 130, 0, 0, 0, 117, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 131, 0, 0, 0, 130, 0, 0, 0, 62, 0, 3, 0, 128, 0, 0, 0, 131, 0, 0, 0, 62, 0, 3, 0, 132, 0, 0, 0, 122, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 127, 0, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 134, 0, 0, 0, 22, 0, 0, 0, 128, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 135, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 136, 0, 0, 0, 119, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 137, 0, 0, 0, 136, 0, 0, 0, 136, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 138, 0, 0, 0, 135, 0, 0, 0, 137, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 139, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 140, 0, 0, 0, 124, 0, 0, 0, 79, 0, 7, 0, 16, 0, 0, 0, 141, 0, 0, 0, 140, 0, 0, 0, 140, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 136, 0, 5, 0, 16, 0, 0, 0, 142, 0, 0, 0, 139, 0, 0, 0, 141, 0, 0, 0, 65, 0, 5, 0, 129, 0, 0, 0, 144, 0, 0, 0, 117, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 145, 0, 0, 0, 144, 0, 0, 0, 62, 0, 3, 0, 143, 0, 0, 0, 145, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 138, 0, 0, 0, 62, 0, 3, 0, 147, 0, 0, 0, 142, 0, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 148, 0, 0, 0, 22, 0, 0, 0, 143, 0, 0, 0, 146, 0, 0, 0, 147, 0, 0, 0, 80, 0, 5, 0, 16, 0, 0, 0, 149, 0, 0, 0, 134, 0, 0, 0, 148, 0, 0, 0, 62, 0, 3, 0, 115, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 153, 0, 0, 0, 156, 0, 0, 0, 155, 0, 0, 0, 61, 0, 4, 0, 157, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 86, 0, 5, 0, 161, 0, 0, 0, 162, 0, 0, 0, 156, 0, 0, 0, 160, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 163, 0, 0, 0, 115, 0, 0, 0, 87, 0, 5, 0, 150, 0, 0, 0, 164, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 61, 0, 4, 0, 150, 0, 0, 0, 167, 0, 0, 0, 166, 0, 0, 0, 133, 0, 5, 0, 150, 0, 0, 0, 168, 0, 0, 0, 164, 0, 0, 0, 167, 0, 0, 0, 62, 0, 3, 0, 152, 0, 0, 0, 168, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 170, 0, 0, 0, 152, 0, 0, 0, 169, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 171, 0, 0, 0, 170, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 173, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 247, 0, 3, 0, 175, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 173, 0, 0, 0, 174, 0, 0, 0, 175, 0, 0, 0, 248, 0, 2, 0, 174, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 175, 0, 0, 0, 61, 0, 4, 0, 150, 0, 0, 0, 179, 0, 0, 0, 152, 0, 0, 0, 62, 0, 3, 0, 178, 0, 0, 0, 179, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 9, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 10, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 11, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 12, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 13, 0, 0, 0, 248, 0, 2, 0, 15, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 24, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 10, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 24, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 27, 0, 0, 0, 11, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 28, 0, 0, 0, 10, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 29, 0, 0, 0, 27, 0, 0, 0, 28, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 30, 0, 0, 0, 26, 0, 0, 0, 29, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 31, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 12, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 31, 0, 0, 0, 32, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 30, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 0, 0, 0, 12, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 34, 0, 0, 0, 35, 0, 0, 0, 254, 0, 2, 0, 36, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 55, 0, 3, 0, 7, 0, 0, 0, 19, 0, 0, 0, 55, 0, 3, 0, 17, 0, 0, 0, 20, 0, 0, 0, 55, 0, 3, 0, 17, 0, 0, 0, 21, 0, 0, 0, 248, 0, 2, 0, 23, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 49, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 55, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 56, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 76, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 78, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 81, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 82, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 85, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 94, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 96, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 97, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 98, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 99, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 39, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 42, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 43, 0, 0, 0, 42, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 45, 0, 0, 0, 39, 0, 0, 0, 43, 0, 0, 0, 247, 0, 3, 0, 47, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 45, 0, 0, 0, 46, 0, 0, 0, 47, 0, 0, 0, 248, 0, 2, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 49, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 48, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 53, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 54, 0, 0, 0, 62, 0, 3, 0, 55, 0, 0, 0, 48, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 57, 0, 0, 0, 20, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 58, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 56, 0, 0, 0, 58, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 59, 0, 0, 0, 14, 0, 0, 0, 49, 0, 0, 0, 51, 0, 0, 0, 52, 0, 0, 0, 55, 0, 0, 0, 56, 0, 0, 0, 254, 0, 2, 0, 59, 0, 0, 0, 248, 0, 2, 0, 47, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 64, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 66, 0, 0, 0, 62, 0, 0, 0, 65, 0, 0, 0, 184, 0, 5, 0, 44, 0, 0, 0, 67, 0, 0, 0, 61, 0, 0, 0, 66, 0, 0, 0, 247, 0, 3, 0, 69, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 67, 0, 0, 0, 68, 0, 0, 0, 69, 0, 0, 0, 248, 0, 2, 0, 68, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 70, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 72, 0, 0, 0, 62, 0, 0, 0, 71, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 73, 0, 0, 0, 20, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 73, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 75, 0, 0, 0, 62, 0, 0, 0, 74, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 76, 0, 0, 0, 77, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 79, 0, 0, 0, 21, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 80, 0, 0, 0, 79, 0, 0, 0, 62, 0, 3, 0, 78, 0, 0, 0, 80, 0, 0, 0, 62, 0, 3, 0, 81, 0, 0, 0, 72, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 83, 0, 0, 0, 20, 0, 0, 0, 41, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 84, 0, 0, 0, 83, 0, 0, 0, 62, 0, 3, 0, 82, 0, 0, 0, 84, 0, 0, 0, 62, 0, 3, 0, 85, 0, 0, 0, 75, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 86, 0, 0, 0, 14, 0, 0, 0, 76, 0, 0, 0, 78, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 85, 0, 0, 0, 254, 0, 2, 0, 86, 0, 0, 0, 248, 0, 2, 0, 69, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 88, 0, 0, 0, 21, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 88, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 90, 0, 0, 0, 62, 0, 0, 0, 89, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 91, 0, 0, 0, 20, 0, 0, 0, 63, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 62, 0, 0, 0, 92, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 19, 0, 0, 0, 62, 0, 3, 0, 94, 0, 0, 0, 95, 0, 0, 0, 62, 0, 3, 0, 96, 0, 0, 0, 90, 0, 0, 0, 62, 0, 3, 0, 97, 0, 0, 0, 62, 0, 0, 0, 62, 0, 3, 0, 98, 0, 0, 0, 93, 0, 0, 0, 62, 0, 3, 0, 99, 0, 0, 0, 62, 0, 0, 0, 57, 0, 9, 0, 6, 0, 0, 0, 100, 0, 0, 0, 14, 0, 0, 0, 94, 0, 0, 0, 96, 0, 0, 0, 97, 0, 0, 0, 98, 0, 0, 0, 99, 0, 0, 0, 254, 0, 2, 0, 100, 0, 0, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_NINEPATCH_VERT[7908] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 45, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 22, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 147, 0, 0, 0, 181, 0, 0, 0, 184, 0, 0, 0, 228, 0, 0, 0, 6, 1, 0, 0, 7, 1, 0, 0, 9, 1, 0, 0, 17, 1, 0, 0, 18, 1, 0, 0, 22, 1, 0, 0, 24, 1, 0, 0, 26, 1, 0, 0, 27, 1, 0, 0, 29, 1, 0, 0, 30, 1, 0, 0, 35, 1, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 3, 0, 8, 0, 0, 0, 113, 120, 120, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 27, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 34, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 40, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 46, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 52, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 59, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 65, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 73, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 5, 0, 141, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 147, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 179, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 181, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 184, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 226, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 228, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 7, 0, 232, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 3, 0, 236, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 241, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 241, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 9, 0, 241, 0, 0, 0, 2, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 3, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 241, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 243, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 6, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 7, 1, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 7, 0, 9, 1, 0, 0, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 4, 0, 17, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 18, 1, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 22, 1, 0, 0, 118, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 24, 1, 0, 0, 105, 95, 109, 97, 114, 103, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 26, 1, 0, 0, 118, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 27, 1, 0, 0, 105, 95, 115, 111, 117, 114, 99, 101, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 29, 1, 0, 0, 118, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 30, 1, 0, 0, 105, 95, 111, 117, 116, 112, 117, 116, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 6, 0, 33, 1, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 33, 1, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 33, 1, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 33, 1, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 33, 1, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 35, 1, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 147, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 181, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 184, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 228, 0, 0, 0, 30, 0, 0, 0, 10, 0, 0, 0, 71, 0, 3, 0, 241, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 6, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 9, 1, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 3, 0, 17, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 17, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 18, 1, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 3, 0, 22, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 22, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 24, 1, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 26, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 26, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 27, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 29, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 29, 1, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 30, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 33, 1, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 33, 1, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 12, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 53, 0, 0, 0, 3, 0, 0, 0, 24, 0, 4, 0, 71, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 72, 0, 0, 0, 7, 0, 0, 0, 71, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 119, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 142, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 143, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 144, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 23, 0, 4, 0, 145, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 146, 0, 0, 0, 1, 0, 0, 0, 145, 0, 0, 0, 59, 0, 4, 0, 146, 0, 0, 0, 147, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 177, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 178, 0, 0, 0, 7, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 180, 0, 0, 0, 1, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 181, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 187, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 188, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 190, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 196, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 203, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 224, 0, 0, 0, 32, 0, 4, 0, 225, 0, 0, 0, 7, 0, 0, 0, 224, 0, 0, 0, 32, 0, 4, 0, 227, 0, 0, 0, 1, 0, 0, 0, 12, 0, 0, 0, 59, 0, 4, 0, 227, 0, 0, 0, 228, 0, 0, 0, 1, 0, 0, 0, 30, 0, 10, 0, 241, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 177, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 242, 0, 0, 0, 2, 0, 0, 0, 241, 0, 0, 0, 59, 0, 4, 0, 242, 0, 0, 0, 243, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 244, 0, 0, 0, 2, 0, 0, 0, 71, 0, 0, 0, 32, 0, 4, 0, 5, 1, 0, 0, 3, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 6, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 7, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 9, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 16, 1, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 17, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 18, 1, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 20, 1, 0, 0, 12, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 21, 1, 0, 0, 3, 0, 0, 0, 20, 1, 0, 0, 59, 0, 4, 0, 21, 1, 0, 0, 22, 1, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 23, 1, 0, 0, 1, 0, 0, 0, 20, 1, 0, 0, 59, 0, 4, 0, 23, 1, 0, 0, 24, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 26, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 27, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 29, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 30, 1, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 32, 1, 0, 0, 6, 0, 0, 0, 21, 0, 0, 0, 30, 0, 6, 0, 33, 1, 0, 0, 9, 0, 0, 0, 6, 0, 0, 0, 32, 1, 0, 0, 32, 1, 0, 0, 32, 0, 4, 0, 34, 1, 0, 0, 3, 0, 0, 0, 33, 1, 0, 0, 59, 0, 4, 0, 34, 1, 0, 0, 35, 1, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 43, 1, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 27, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 34, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 59, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 73, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 141, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 178, 0, 0, 0, 179, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 226, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 232, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 236, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 238, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 249, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 15, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 17, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 18, 0, 0, 0, 17, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 19, 0, 0, 0, 16, 0, 0, 0, 18, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 22, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 24, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 25, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 29, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 31, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 31, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 30, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 27, 0, 0, 0, 33, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 35, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 37, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 34, 0, 0, 0, 39, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 41, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 43, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 42, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 45, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 47, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 49, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 51, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 54, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 56, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 60, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 62, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 59, 0, 0, 0, 64, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 66, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 68, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 70, 0, 0, 0, 67, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 79, 0, 0, 0, 75, 0, 0, 0, 78, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 80, 0, 0, 0, 74, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 81, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 65, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 84, 0, 0, 0, 75, 0, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 59, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 87, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 88, 0, 0, 0, 75, 0, 0, 0, 87, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 90, 0, 0, 0, 80, 0, 0, 0, 84, 0, 0, 0, 88, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 65, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 91, 0, 0, 0, 92, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 75, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 97, 0, 0, 0, 95, 0, 0, 0, 96, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 98, 0, 0, 0, 75, 0, 0, 0, 97, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 99, 0, 0, 0, 74, 0, 0, 0, 98, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 100, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 52, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 103, 0, 0, 0, 75, 0, 0, 0, 102, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 104, 0, 0, 0, 94, 0, 0, 0, 99, 0, 0, 0, 103, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 105, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 59, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 108, 0, 0, 0, 75, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 111, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 112, 0, 0, 0, 75, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 113, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 20, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 114, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 116, 0, 0, 0, 75, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 117, 0, 0, 0, 74, 0, 0, 0, 116, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 118, 0, 0, 0, 108, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 89, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 120, 0, 0, 0, 90, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 121, 0, 0, 0, 90, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 122, 0, 0, 0, 90, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 123, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 124, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 125, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 126, 0, 0, 0, 104, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 104, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 128, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 130, 0, 0, 0, 118, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 131, 0, 0, 0, 118, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 132, 0, 0, 0, 119, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 133, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 134, 0, 0, 0, 119, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 135, 0, 0, 0, 119, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 136, 0, 0, 0, 120, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 123, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 137, 0, 0, 0, 124, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 138, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 139, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 140, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 139, 0, 0, 0, 62, 0, 3, 0, 73, 0, 0, 0, 140, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 150, 0, 0, 0, 147, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 152, 0, 0, 0, 149, 0, 0, 0, 151, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 153, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 154, 0, 0, 0, 142, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 155, 0, 0, 0, 142, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 156, 0, 0, 0, 142, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 157, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 158, 0, 0, 0, 143, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 159, 0, 0, 0, 143, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 143, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 161, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 162, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 163, 0, 0, 0, 144, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 164, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 165, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 166, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 167, 0, 0, 0, 152, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 168, 0, 0, 0, 152, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 169, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 156, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 170, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 159, 0, 0, 0, 160, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 171, 0, 0, 0, 161, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 164, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 172, 0, 0, 0, 165, 0, 0, 0, 166, 0, 0, 0, 167, 0, 0, 0, 168, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 173, 0, 0, 0, 169, 0, 0, 0, 170, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 173, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 174, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 175, 0, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 176, 0, 0, 0, 175, 0, 0, 0, 174, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 176, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 182, 0, 0, 0, 181, 0, 0, 0, 127, 0, 4, 0, 177, 0, 0, 0, 183, 0, 0, 0, 182, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 185, 0, 0, 0, 184, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 186, 0, 0, 0, 183, 0, 0, 0, 185, 0, 0, 0, 62, 0, 3, 0, 179, 0, 0, 0, 186, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 191, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 192, 0, 0, 0, 191, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 193, 0, 0, 0, 179, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 194, 0, 0, 0, 193, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 195, 0, 0, 0, 192, 0, 0, 0, 194, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 197, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 198, 0, 0, 0, 197, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 199, 0, 0, 0, 179, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 200, 0, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 201, 0, 0, 0, 198, 0, 0, 0, 200, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 202, 0, 0, 0, 195, 0, 0, 0, 201, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 204, 0, 0, 0, 141, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 89, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 207, 0, 0, 0, 202, 0, 0, 0, 206, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 208, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 209, 0, 0, 0, 208, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 210, 0, 0, 0, 207, 0, 0, 0, 209, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 211, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 62, 0, 3, 0, 211, 0, 0, 0, 210, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 212, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 213, 0, 0, 0, 212, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 214, 0, 0, 0, 184, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 215, 0, 0, 0, 214, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 216, 0, 0, 0, 213, 0, 0, 0, 215, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 217, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 217, 0, 0, 0, 216, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 218, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 219, 0, 0, 0, 218, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 220, 0, 0, 0, 184, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 221, 0, 0, 0, 220, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 222, 0, 0, 0, 219, 0, 0, 0, 221, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 223, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 62, 0, 3, 0, 223, 0, 0, 0, 222, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 229, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 230, 0, 0, 0, 229, 0, 0, 0, 21, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 231, 0, 0, 0, 230, 0, 0, 0, 21, 0, 0, 0, 62, 0, 3, 0, 226, 0, 0, 0, 231, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 233, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 28, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 232, 0, 0, 0, 235, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 237, 0, 0, 0, 226, 0, 0, 0, 247, 0, 3, 0, 240, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 237, 0, 0, 0, 239, 0, 0, 0, 247, 0, 0, 0, 248, 0, 2, 0, 239, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 245, 0, 0, 0, 243, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 246, 0, 0, 0, 245, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 246, 0, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 247, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 248, 0, 0, 0, 232, 0, 0, 0, 247, 0, 3, 0, 251, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 248, 0, 0, 0, 250, 0, 0, 0, 254, 0, 0, 0, 248, 0, 2, 0, 250, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 252, 0, 0, 0, 243, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 253, 0, 0, 0, 252, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 253, 0, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 254, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 255, 0, 0, 0, 243, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 0, 1, 0, 0, 255, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 0, 1, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 251, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 1, 1, 0, 0, 249, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 1, 1, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 240, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 2, 1, 0, 0, 238, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 3, 1, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 4, 1, 0, 0, 2, 1, 0, 0, 3, 1, 0, 0, 62, 0, 3, 0, 236, 0, 0, 0, 4, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 10, 1, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 12, 1, 0, 0, 8, 1, 0, 0, 11, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 13, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 14, 1, 0, 0, 13, 1, 0, 0, 13, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 177, 0, 0, 0, 15, 1, 0, 0, 12, 1, 0, 0, 14, 1, 0, 0, 62, 0, 3, 0, 6, 1, 0, 0, 15, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 19, 1, 0, 0, 18, 1, 0, 0, 62, 0, 3, 0, 17, 1, 0, 0, 19, 1, 0, 0, 61, 0, 4, 0, 20, 1, 0, 0, 25, 1, 0, 0, 24, 1, 0, 0, 62, 0, 3, 0, 22, 1, 0, 0, 25, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 28, 1, 0, 0, 27, 1, 0, 0, 62, 0, 3, 0, 26, 1, 0, 0, 28, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 31, 1, 0, 0, 30, 1, 0, 0, 62, 0, 3, 0, 29, 1, 0, 0, 31, 1, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 36, 1, 0, 0, 236, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 37, 1, 0, 0, 7, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 38, 1, 0, 0, 37, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 39, 1, 0, 0, 37, 1, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 40, 1, 0, 0, 38, 1, 0, 0, 39, 1, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 145, 0, 5, 0, 9, 0, 0, 0, 41, 1, 0, 0, 36, 1, 0, 0, 40, 1, 0, 0, 65, 0, 5, 0, 16, 1, 0, 0, 42, 1, 0, 0, 35, 1, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 42, 1, 0, 0, 41, 1, 0, 0, 65, 0, 6, 0, 43, 1, 0, 0, 44, 1, 0, 0, 35, 1, 0, 0, 189, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 44, 1, 0, 0, 74, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SHAPE_FRAG[14092] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 78, 2, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 14, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 53, 1, 0, 0, 56, 1, 0, 0, 63, 1, 0, 0, 70, 1, 0, 0, 82, 1, 0, 0, 123, 1, 0, 0, 165, 1, 0, 0, 254, 1, 0, 0, 75, 2, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 7, 0, 13, 0, 0, 0, 115, 100, 102, 95, 99, 105, 114, 99, 108, 101, 40, 118, 102, 50, 59, 102, 49, 59, 0, 0, 5, 0, 3, 0, 11, 0, 0, 0, 112, 0, 0, 0, 5, 0, 4, 0, 12, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 5, 0, 18, 0, 0, 0, 109, 111, 100, 98, 40, 102, 49, 59, 102, 49, 59, 0, 5, 0, 3, 0, 16, 0, 0, 0, 120, 0, 0, 0, 5, 0, 3, 0, 17, 0, 0, 0, 121, 0, 0, 0, 5, 0, 8, 0, 25, 0, 0, 0, 97, 114, 99, 95, 115, 100, 102, 40, 118, 102, 50, 59, 102, 49, 59, 102, 49, 59, 102, 49, 59, 0, 0, 0, 5, 0, 3, 0, 21, 0, 0, 0, 112, 0, 0, 0, 5, 0, 3, 0, 22, 0, 0, 0, 97, 48, 0, 0, 5, 0, 3, 0, 23, 0, 0, 0, 97, 49, 0, 0, 5, 0, 3, 0, 24, 0, 0, 0, 114, 0, 0, 0, 5, 0, 9, 0, 33, 0, 0, 0, 115, 100, 95, 114, 111, 117, 110, 100, 101, 100, 95, 98, 111, 120, 40, 118, 102, 50, 59, 118, 102, 50, 59, 118, 102, 52, 59, 0, 5, 0, 4, 0, 30, 0, 0, 0, 112, 111, 105, 110, 116, 0, 0, 0, 5, 0, 4, 0, 31, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 6, 0, 32, 0, 0, 0, 99, 111, 114, 110, 101, 114, 95, 114, 97, 100, 105, 105, 0, 0, 0, 0, 5, 0, 12, 0, 40, 0, 0, 0, 115, 100, 95, 105, 110, 115, 101, 116, 95, 114, 111, 117, 110, 100, 101, 100, 95, 98, 111, 120, 40, 118, 102, 50, 59, 118, 102, 50, 59, 118, 102, 52, 59, 118, 102, 52, 59, 0, 0, 0, 5, 0, 4, 0, 36, 0, 0, 0, 112, 111, 105, 110, 116, 0, 0, 0, 5, 0, 4, 0, 37, 0, 0, 0, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 4, 0, 38, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 4, 0, 39, 0, 0, 0, 105, 110, 115, 101, 116, 0, 0, 0, 5, 0, 6, 0, 44, 0, 0, 0, 97, 110, 116, 105, 97, 108, 105, 97, 115, 40, 102, 49, 59, 0, 0, 0, 5, 0, 3, 0, 43, 0, 0, 0, 100, 0, 0, 0, 5, 0, 8, 0, 47, 0, 0, 0, 97, 110, 116, 105, 97, 108, 105, 97, 115, 95, 99, 105, 114, 99, 108, 101, 40, 102, 49, 59, 0, 0, 0, 0, 5, 0, 3, 0, 46, 0, 0, 0, 100, 0, 0, 0, 5, 0, 3, 0, 65, 0, 0, 0, 97, 0, 0, 0, 5, 0, 4, 0, 75, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 76, 0, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 3, 0, 78, 0, 0, 0, 97, 112, 0, 0, 5, 0, 3, 0, 90, 0, 0, 0, 97, 49, 112, 0, 5, 0, 3, 0, 105, 0, 0, 0, 113, 48, 0, 0, 5, 0, 3, 0, 115, 0, 0, 0, 113, 49, 0, 0, 5, 0, 3, 0, 142, 0, 0, 0, 114, 115, 0, 0, 5, 0, 4, 0, 155, 0, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 6, 0, 168, 0, 0, 0, 99, 111, 114, 110, 101, 114, 95, 116, 111, 95, 112, 111, 105, 110, 116, 0, 5, 0, 3, 0, 175, 0, 0, 0, 113, 0, 0, 0, 5, 0, 3, 0, 180, 0, 0, 0, 108, 0, 0, 0, 5, 0, 3, 0, 185, 0, 0, 0, 109, 0, 0, 0, 5, 0, 5, 0, 199, 0, 0, 0, 105, 110, 110, 101, 114, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 207, 0, 0, 0, 105, 110, 110, 101, 114, 95, 99, 101, 110, 116, 101, 114, 0, 0, 0, 0, 5, 0, 5, 0, 216, 0, 0, 0, 105, 110, 110, 101, 114, 95, 112, 111, 105, 110, 116, 0, 5, 0, 3, 0, 220, 0, 0, 0, 114, 0, 0, 0, 5, 0, 5, 0, 4, 1, 0, 0, 104, 97, 108, 102, 95, 115, 105, 122, 101, 0, 0, 0, 5, 0, 5, 0, 7, 1, 0, 0, 109, 105, 110, 95, 115, 105, 122, 101, 0, 0, 0, 0, 5, 0, 4, 0, 19, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 21, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 23, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 28, 1, 0, 0, 97, 102, 119, 105, 100, 116, 104, 0, 5, 0, 4, 0, 40, 1, 0, 0, 97, 102, 119, 105, 100, 116, 104, 0, 5, 0, 4, 0, 51, 1, 0, 0, 114, 101, 115, 117, 108, 116, 0, 0, 5, 0, 4, 0, 53, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 56, 1, 0, 0, 118, 95, 115, 104, 97, 112, 101, 0, 5, 0, 4, 0, 61, 1, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 4, 0, 63, 1, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 7, 0, 69, 1, 0, 0, 101, 120, 116, 101, 114, 110, 97, 108, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 0, 0, 5, 0, 4, 0, 70, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 4, 0, 74, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 75, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 78, 1, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 4, 0, 79, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 7, 0, 82, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 7, 0, 87, 1, 0, 0, 105, 110, 116, 101, 114, 110, 97, 108, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 0, 0, 5, 0, 4, 0, 97, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 98, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 6, 0, 100, 1, 0, 0, 98, 111, 114, 100, 101, 114, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 6, 0, 105, 1, 0, 0, 98, 111, 114, 100, 101, 114, 95, 97, 108, 112, 104, 97, 0, 0, 0, 0, 5, 0, 4, 0, 106, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 109, 1, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 7, 0, 121, 1, 0, 0, 99, 111, 108, 111, 114, 95, 119, 105, 116, 104, 95, 98, 111, 114, 100, 101, 114, 0, 0, 0, 5, 0, 6, 0, 123, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 5, 0, 164, 1, 0, 0, 115, 116, 97, 114, 116, 95, 97, 110, 103, 108, 101, 0, 5, 0, 6, 0, 165, 1, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 5, 0, 168, 1, 0, 0, 101, 110, 100, 95, 97, 110, 103, 108, 101, 0, 0, 0, 5, 0, 5, 0, 171, 1, 0, 0, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 0, 5, 0, 3, 0, 177, 1, 0, 0, 112, 0, 0, 0, 5, 0, 3, 0, 194, 1, 0, 0, 100, 0, 0, 0, 5, 0, 4, 0, 197, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 199, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 201, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 203, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 3, 0, 205, 1, 0, 0, 97, 97, 0, 0, 5, 0, 4, 0, 212, 1, 0, 0, 97, 108, 112, 104, 97, 0, 0, 0, 5, 0, 4, 0, 231, 1, 0, 0, 114, 97, 100, 105, 117, 115, 0, 0, 5, 0, 7, 0, 253, 1, 0, 0, 101, 120, 116, 101, 114, 110, 97, 108, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 0, 0, 5, 0, 4, 0, 254, 1, 0, 0, 118, 95, 112, 111, 105, 110, 116, 0, 5, 0, 4, 0, 255, 1, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 1, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 3, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 7, 0, 6, 2, 0, 0, 105, 110, 116, 101, 114, 110, 97, 108, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 0, 0, 5, 0, 4, 0, 9, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 11, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 13, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 4, 0, 15, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 6, 0, 17, 2, 0, 0, 98, 111, 114, 100, 101, 114, 95, 100, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 6, 0, 22, 2, 0, 0, 98, 111, 114, 100, 101, 114, 95, 97, 108, 112, 104, 97, 0, 0, 0, 0, 5, 0, 4, 0, 23, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 6, 0, 26, 2, 0, 0, 115, 109, 111, 111, 116, 104, 101, 100, 95, 97, 108, 112, 104, 97, 0, 0, 5, 0, 4, 0, 27, 2, 0, 0, 112, 97, 114, 97, 109, 0, 0, 0, 5, 0, 5, 0, 30, 2, 0, 0, 113, 117, 97, 100, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 8, 0, 41, 2, 0, 0, 113, 117, 97, 100, 95, 99, 111, 108, 111, 114, 95, 119, 105, 116, 104, 95, 98, 111, 114, 100, 101, 114, 0, 0, 5, 0, 5, 0, 75, 2, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 3, 0, 53, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 53, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 3, 0, 56, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 56, 1, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 63, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 63, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 70, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 82, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 82, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 123, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 123, 1, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 165, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 165, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 254, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 75, 2, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 9, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 33, 0, 5, 0, 10, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 33, 0, 5, 0, 15, 0, 0, 0, 6, 0, 0, 0, 9, 0, 0, 0, 9, 0, 0, 0, 33, 0, 7, 0, 20, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 9, 0, 0, 0, 9, 0, 0, 0, 23, 0, 4, 0, 27, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 7, 0, 0, 0, 27, 0, 0, 0, 33, 0, 6, 0, 29, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 28, 0, 0, 0, 33, 0, 7, 0, 35, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 28, 0, 0, 0, 28, 0, 0, 0, 33, 0, 4, 0, 42, 0, 0, 0, 6, 0, 0, 0, 9, 0, 0, 0, 21, 0, 4, 0, 66, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 66, 0, 0, 0, 67, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 66, 0, 0, 0, 70, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 219, 15, 201, 64, 43, 0, 4, 0, 6, 0, 0, 0, 83, 0, 0, 0, 0, 0, 0, 0, 20, 0, 2, 0, 84, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 171, 0, 0, 0, 0, 0, 0, 63, 44, 0, 5, 0, 7, 0, 0, 0, 182, 0, 0, 0, 83, 0, 0, 0, 83, 0, 0, 0, 43, 0, 4, 0, 66, 0, 0, 0, 233, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 66, 0, 0, 0, 245, 0, 0, 0, 3, 0, 0, 0, 44, 0, 7, 0, 27, 0, 0, 0, 14, 1, 0, 0, 83, 0, 0, 0, 83, 0, 0, 0, 83, 0, 0, 0, 83, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 31, 1, 0, 0, 47, 186, 232, 62, 43, 0, 4, 0, 6, 0, 0, 0, 33, 1, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 43, 1, 0, 0, 102, 102, 166, 63, 32, 0, 4, 0, 52, 1, 0, 0, 1, 0, 0, 0, 27, 0, 0, 0, 59, 0, 4, 0, 52, 1, 0, 0, 53, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 55, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 59, 0, 4, 0, 55, 1, 0, 0, 56, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 62, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 62, 1, 0, 0, 63, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 64, 1, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 62, 1, 0, 0, 70, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 64, 1, 0, 0, 82, 1, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 110, 1, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 52, 1, 0, 0, 123, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 52, 1, 0, 0, 165, 1, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 184, 1, 0, 0, 0, 0, 0, 64, 59, 0, 4, 0, 62, 1, 0, 0, 254, 1, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 69, 2, 0, 0, 10, 215, 35, 60, 32, 0, 4, 0, 74, 2, 0, 0, 3, 0, 0, 0, 27, 0, 0, 0, 59, 0, 4, 0, 74, 2, 0, 0, 75, 2, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 77, 2, 0, 0, 219, 15, 73, 64, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 51, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 61, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 69, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 74, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 75, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 78, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 79, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 87, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 97, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 98, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 100, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 105, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 106, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 109, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 121, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 138, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 164, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 168, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 171, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 177, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 194, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 197, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 199, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 201, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 203, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 205, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 212, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 231, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 253, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 255, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 1, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 3, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 6, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 9, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 11, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 13, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 15, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 17, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 22, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 23, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 26, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 27, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 30, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 41, 2, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 57, 2, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 54, 1, 0, 0, 53, 1, 0, 0, 62, 0, 3, 0, 51, 1, 0, 0, 54, 1, 0, 0, 61, 0, 4, 0, 66, 0, 0, 0, 57, 1, 0, 0, 56, 1, 0, 0, 170, 0, 5, 0, 84, 0, 0, 0, 58, 1, 0, 0, 57, 1, 0, 0, 67, 0, 0, 0, 247, 0, 3, 0, 60, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 58, 1, 0, 0, 59, 1, 0, 0, 159, 1, 0, 0, 248, 0, 2, 0, 59, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 65, 1, 0, 0, 63, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 66, 1, 0, 0, 65, 1, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 67, 1, 0, 0, 33, 1, 0, 0, 66, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 68, 1, 0, 0, 171, 0, 0, 0, 67, 1, 0, 0, 62, 0, 3, 0, 61, 1, 0, 0, 68, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 71, 1, 0, 0, 70, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 72, 1, 0, 0, 171, 0, 0, 0, 171, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 73, 1, 0, 0, 71, 1, 0, 0, 72, 1, 0, 0, 62, 0, 3, 0, 74, 1, 0, 0, 73, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 1, 0, 0, 61, 1, 0, 0, 62, 0, 3, 0, 75, 1, 0, 0, 76, 1, 0, 0, 57, 0, 6, 0, 6, 0, 0, 0, 77, 1, 0, 0, 13, 0, 0, 0, 74, 1, 0, 0, 75, 1, 0, 0, 62, 0, 3, 0, 69, 1, 0, 0, 77, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 80, 1, 0, 0, 69, 1, 0, 0, 62, 0, 3, 0, 79, 1, 0, 0, 80, 1, 0, 0, 57, 0, 5, 0, 6, 0, 0, 0, 81, 1, 0, 0, 47, 0, 0, 0, 79, 1, 0, 0, 62, 0, 3, 0, 78, 1, 0, 0, 81, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 83, 1, 0, 0, 82, 1, 0, 0, 186, 0, 5, 0, 84, 0, 0, 0, 84, 1, 0, 0, 83, 1, 0, 0, 83, 0, 0, 0, 247, 0, 3, 0, 86, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 84, 1, 0, 0, 85, 1, 0, 0, 148, 1, 0, 0, 248, 0, 2, 0, 85, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 88, 1, 0, 0, 70, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 89, 1, 0, 0, 171, 0, 0, 0, 171, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 90, 1, 0, 0, 88, 1, 0, 0, 89, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 1, 0, 0, 61, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 1, 0, 0, 82, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 93, 1, 0, 0, 63, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 94, 1, 0, 0, 93, 1, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 95, 1, 0, 0, 92, 1, 0, 0, 94, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 96, 1, 0, 0, 91, 1, 0, 0, 95, 1, 0, 0, 62, 0, 3, 0, 97, 1, 0, 0, 90, 1, 0, 0, 62, 0, 3, 0, 98, 1, 0, 0, 96, 1, 0, 0, 57, 0, 6, 0, 6, 0, 0, 0, 99, 1, 0, 0, 13, 0, 0, 0, 97, 1, 0, 0, 98, 1, 0, 0, 62, 0, 3, 0, 87, 1, 0, 0, 99, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 1, 0, 0, 69, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 102, 1, 0, 0, 87, 1, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 103, 1, 0, 0, 102, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 104, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 101, 1, 0, 0, 103, 1, 0, 0, 62, 0, 3, 0, 100, 1, 0, 0, 104, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 107, 1, 0, 0, 100, 1, 0, 0, 62, 0, 3, 0, 106, 1, 0, 0, 107, 1, 0, 0, 57, 0, 5, 0, 6, 0, 0, 0, 108, 1, 0, 0, 47, 0, 0, 0, 106, 1, 0, 0, 62, 0, 3, 0, 105, 1, 0, 0, 108, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 111, 1, 0, 0, 53, 1, 0, 0, 79, 0, 8, 0, 110, 1, 0, 0, 112, 1, 0, 0, 111, 1, 0, 0, 111, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 113, 1, 0, 0, 53, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 1, 0, 0, 113, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 115, 1, 0, 0, 78, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 116, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 114, 1, 0, 0, 115, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 117, 1, 0, 0, 112, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 118, 1, 0, 0, 112, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 119, 1, 0, 0, 112, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 120, 1, 0, 0, 117, 1, 0, 0, 118, 1, 0, 0, 119, 1, 0, 0, 116, 1, 0, 0, 62, 0, 3, 0, 109, 1, 0, 0, 120, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 122, 1, 0, 0, 109, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 124, 1, 0, 0, 123, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 125, 1, 0, 0, 123, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 126, 1, 0, 0, 125, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 127, 1, 0, 0, 105, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 128, 1, 0, 0, 78, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 129, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 127, 1, 0, 0, 128, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 130, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 126, 1, 0, 0, 129, 1, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 131, 1, 0, 0, 130, 1, 0, 0, 130, 1, 0, 0, 130, 1, 0, 0, 130, 1, 0, 0, 12, 0, 8, 0, 27, 0, 0, 0, 132, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 122, 1, 0, 0, 124, 1, 0, 0, 131, 1, 0, 0, 62, 0, 3, 0, 121, 1, 0, 0, 132, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 133, 1, 0, 0, 87, 1, 0, 0, 186, 0, 5, 0, 84, 0, 0, 0, 134, 1, 0, 0, 133, 1, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 135, 1, 0, 0, 105, 1, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 136, 1, 0, 0, 135, 1, 0, 0, 33, 1, 0, 0, 167, 0, 5, 0, 84, 0, 0, 0, 137, 1, 0, 0, 134, 1, 0, 0, 136, 1, 0, 0, 247, 0, 3, 0, 140, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 137, 1, 0, 0, 139, 1, 0, 0, 145, 1, 0, 0, 248, 0, 2, 0, 139, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 141, 1, 0, 0, 123, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 142, 1, 0, 0, 105, 1, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 143, 1, 0, 0, 142, 1, 0, 0, 142, 1, 0, 0, 142, 1, 0, 0, 142, 1, 0, 0, 12, 0, 8, 0, 27, 0, 0, 0, 144, 1, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 14, 1, 0, 0, 141, 1, 0, 0, 143, 1, 0, 0, 62, 0, 3, 0, 138, 1, 0, 0, 144, 1, 0, 0, 249, 0, 2, 0, 140, 1, 0, 0, 248, 0, 2, 0, 145, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 146, 1, 0, 0, 121, 1, 0, 0, 62, 0, 3, 0, 138, 1, 0, 0, 146, 1, 0, 0, 249, 0, 2, 0, 140, 1, 0, 0, 248, 0, 2, 0, 140, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 147, 1, 0, 0, 138, 1, 0, 0, 62, 0, 3, 0, 51, 1, 0, 0, 147, 1, 0, 0, 249, 0, 2, 0, 86, 1, 0, 0, 248, 0, 2, 0, 148, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 149, 1, 0, 0, 53, 1, 0, 0, 79, 0, 8, 0, 110, 1, 0, 0, 150, 1, 0, 0, 149, 1, 0, 0, 149, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 151, 1, 0, 0, 53, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 152, 1, 0, 0, 151, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 153, 1, 0, 0, 78, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 154, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 152, 1, 0, 0, 153, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 155, 1, 0, 0, 150, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 156, 1, 0, 0, 150, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 157, 1, 0, 0, 150, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 158, 1, 0, 0, 155, 1, 0, 0, 156, 1, 0, 0, 157, 1, 0, 0, 154, 1, 0, 0, 62, 0, 3, 0, 51, 1, 0, 0, 158, 1, 0, 0, 249, 0, 2, 0, 86, 1, 0, 0, 248, 0, 2, 0, 86, 1, 0, 0, 249, 0, 2, 0, 60, 1, 0, 0, 248, 0, 2, 0, 159, 1, 0, 0, 61, 0, 4, 0, 66, 0, 0, 0, 160, 1, 0, 0, 56, 1, 0, 0, 170, 0, 5, 0, 84, 0, 0, 0, 161, 1, 0, 0, 160, 1, 0, 0, 233, 0, 0, 0, 247, 0, 3, 0, 163, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 161, 1, 0, 0, 162, 1, 0, 0, 230, 1, 0, 0, 248, 0, 2, 0, 162, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 166, 1, 0, 0, 165, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 167, 1, 0, 0, 166, 1, 0, 0, 62, 0, 3, 0, 164, 1, 0, 0, 167, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 169, 1, 0, 0, 165, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 170, 1, 0, 0, 169, 1, 0, 0, 62, 0, 3, 0, 168, 1, 0, 0, 170, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 172, 1, 0, 0, 82, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 173, 1, 0, 0, 63, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 174, 1, 0, 0, 173, 1, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 175, 1, 0, 0, 172, 1, 0, 0, 174, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 176, 1, 0, 0, 171, 0, 0, 0, 175, 1, 0, 0, 62, 0, 3, 0, 171, 1, 0, 0, 176, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 178, 1, 0, 0, 70, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 179, 1, 0, 0, 178, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 180, 1, 0, 0, 70, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 181, 1, 0, 0, 180, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 182, 1, 0, 0, 33, 1, 0, 0, 181, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 183, 1, 0, 0, 179, 1, 0, 0, 182, 1, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 185, 1, 0, 0, 183, 1, 0, 0, 184, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 186, 1, 0, 0, 33, 1, 0, 0, 33, 1, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 187, 1, 0, 0, 185, 1, 0, 0, 186, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 188, 1, 0, 0, 63, 1, 0, 0, 133, 0, 5, 0, 7, 0, 0, 0, 189, 1, 0, 0, 187, 1, 0, 0, 188, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 190, 1, 0, 0, 63, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 191, 1, 0, 0, 190, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 192, 1, 0, 0, 191, 1, 0, 0, 191, 1, 0, 0, 136, 0, 5, 0, 7, 0, 0, 0, 193, 1, 0, 0, 189, 1, 0, 0, 192, 1, 0, 0, 62, 0, 3, 0, 177, 1, 0, 0, 193, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 195, 1, 0, 0, 171, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 196, 1, 0, 0, 33, 1, 0, 0, 195, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 198, 1, 0, 0, 177, 1, 0, 0, 62, 0, 3, 0, 197, 1, 0, 0, 198, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 200, 1, 0, 0, 164, 1, 0, 0, 62, 0, 3, 0, 199, 1, 0, 0, 200, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 202, 1, 0, 0, 168, 1, 0, 0, 62, 0, 3, 0, 201, 1, 0, 0, 202, 1, 0, 0, 62, 0, 3, 0, 203, 1, 0, 0, 196, 1, 0, 0, 57, 0, 8, 0, 6, 0, 0, 0, 204, 1, 0, 0, 25, 0, 0, 0, 197, 1, 0, 0, 199, 1, 0, 0, 201, 1, 0, 0, 203, 1, 0, 0, 62, 0, 3, 0, 194, 1, 0, 0, 204, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 206, 1, 0, 0, 194, 1, 0, 0, 207, 0, 4, 0, 6, 0, 0, 0, 207, 1, 0, 0, 206, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 208, 1, 0, 0, 194, 1, 0, 0, 208, 0, 4, 0, 6, 0, 0, 0, 209, 1, 0, 0, 208, 1, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 210, 1, 0, 0, 207, 1, 0, 0, 209, 1, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 211, 1, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 210, 1, 0, 0, 62, 0, 3, 0, 205, 1, 0, 0, 211, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 213, 1, 0, 0, 171, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 214, 1, 0, 0, 205, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 215, 1, 0, 0, 213, 1, 0, 0, 214, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 216, 1, 0, 0, 171, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 217, 1, 0, 0, 194, 1, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 218, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 215, 1, 0, 0, 216, 1, 0, 0, 217, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 219, 1, 0, 0, 33, 1, 0, 0, 218, 1, 0, 0, 62, 0, 3, 0, 212, 1, 0, 0, 219, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 220, 1, 0, 0, 53, 1, 0, 0, 79, 0, 8, 0, 110, 1, 0, 0, 221, 1, 0, 0, 220, 1, 0, 0, 220, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 222, 1, 0, 0, 53, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 223, 1, 0, 0, 222, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 224, 1, 0, 0, 212, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 225, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 223, 1, 0, 0, 224, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 226, 1, 0, 0, 221, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 227, 1, 0, 0, 221, 1, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 228, 1, 0, 0, 221, 1, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 229, 1, 0, 0, 226, 1, 0, 0, 227, 1, 0, 0, 228, 1, 0, 0, 225, 1, 0, 0, 62, 0, 3, 0, 51, 1, 0, 0, 229, 1, 0, 0, 249, 0, 2, 0, 163, 1, 0, 0, 248, 0, 2, 0, 230, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 232, 1, 0, 0, 165, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 233, 1, 0, 0, 232, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 234, 1, 0, 0, 165, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 235, 1, 0, 0, 234, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 236, 1, 0, 0, 165, 1, 0, 0, 233, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 237, 1, 0, 0, 236, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 238, 1, 0, 0, 165, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 239, 1, 0, 0, 238, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 240, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 237, 1, 0, 0, 239, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 241, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 235, 1, 0, 0, 240, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 242, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 233, 1, 0, 0, 241, 1, 0, 0, 62, 0, 3, 0, 231, 1, 0, 0, 242, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 243, 1, 0, 0, 231, 1, 0, 0, 186, 0, 5, 0, 84, 0, 0, 0, 244, 1, 0, 0, 243, 1, 0, 0, 83, 0, 0, 0, 168, 0, 4, 0, 84, 0, 0, 0, 245, 1, 0, 0, 244, 1, 0, 0, 247, 0, 3, 0, 247, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 245, 1, 0, 0, 246, 1, 0, 0, 247, 1, 0, 0, 248, 0, 2, 0, 246, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 248, 1, 0, 0, 82, 1, 0, 0, 186, 0, 5, 0, 84, 0, 0, 0, 249, 1, 0, 0, 248, 1, 0, 0, 83, 0, 0, 0, 249, 0, 2, 0, 247, 1, 0, 0, 248, 0, 2, 0, 247, 1, 0, 0, 245, 0, 7, 0, 84, 0, 0, 0, 250, 1, 0, 0, 244, 1, 0, 0, 230, 1, 0, 0, 249, 1, 0, 0, 246, 1, 0, 0, 247, 0, 3, 0, 252, 1, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 250, 1, 0, 0, 251, 1, 0, 0, 252, 1, 0, 0, 248, 0, 2, 0, 251, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 0, 2, 0, 0, 254, 1, 0, 0, 62, 0, 3, 0, 255, 1, 0, 0, 0, 2, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 2, 2, 0, 0, 63, 1, 0, 0, 62, 0, 3, 0, 1, 2, 0, 0, 2, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 4, 2, 0, 0, 165, 1, 0, 0, 62, 0, 3, 0, 3, 2, 0, 0, 4, 2, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 5, 2, 0, 0, 33, 0, 0, 0, 255, 1, 0, 0, 1, 2, 0, 0, 3, 2, 0, 0, 62, 0, 3, 0, 253, 1, 0, 0, 5, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 7, 2, 0, 0, 82, 1, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 8, 2, 0, 0, 7, 2, 0, 0, 7, 2, 0, 0, 7, 2, 0, 0, 7, 2, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 10, 2, 0, 0, 254, 1, 0, 0, 62, 0, 3, 0, 9, 2, 0, 0, 10, 2, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 12, 2, 0, 0, 63, 1, 0, 0, 62, 0, 3, 0, 11, 2, 0, 0, 12, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 14, 2, 0, 0, 165, 1, 0, 0, 62, 0, 3, 0, 13, 2, 0, 0, 14, 2, 0, 0, 62, 0, 3, 0, 15, 2, 0, 0, 8, 2, 0, 0, 57, 0, 8, 0, 6, 0, 0, 0, 16, 2, 0, 0, 40, 0, 0, 0, 9, 2, 0, 0, 11, 2, 0, 0, 13, 2, 0, 0, 15, 2, 0, 0, 62, 0, 3, 0, 6, 2, 0, 0, 16, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 18, 2, 0, 0, 253, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 19, 2, 0, 0, 6, 2, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 20, 2, 0, 0, 19, 2, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 21, 2, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 18, 2, 0, 0, 20, 2, 0, 0, 62, 0, 3, 0, 17, 2, 0, 0, 21, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 24, 2, 0, 0, 17, 2, 0, 0, 62, 0, 3, 0, 23, 2, 0, 0, 24, 2, 0, 0, 57, 0, 5, 0, 6, 0, 0, 0, 25, 2, 0, 0, 44, 0, 0, 0, 23, 2, 0, 0, 62, 0, 3, 0, 22, 2, 0, 0, 25, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 28, 2, 0, 0, 253, 1, 0, 0, 62, 0, 3, 0, 27, 2, 0, 0, 28, 2, 0, 0, 57, 0, 5, 0, 6, 0, 0, 0, 29, 2, 0, 0, 44, 0, 0, 0, 27, 2, 0, 0, 62, 0, 3, 0, 26, 2, 0, 0, 29, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 31, 2, 0, 0, 53, 1, 0, 0, 79, 0, 8, 0, 110, 1, 0, 0, 32, 2, 0, 0, 31, 2, 0, 0, 31, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 33, 2, 0, 0, 53, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 34, 2, 0, 0, 33, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 2, 0, 0, 26, 2, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 36, 2, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 34, 2, 0, 0, 35, 2, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 37, 2, 0, 0, 32, 2, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 38, 2, 0, 0, 32, 2, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 39, 2, 0, 0, 32, 2, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 40, 2, 0, 0, 37, 2, 0, 0, 38, 2, 0, 0, 39, 2, 0, 0, 36, 2, 0, 0, 62, 0, 3, 0, 30, 2, 0, 0, 40, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 42, 2, 0, 0, 30, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 43, 2, 0, 0, 123, 1, 0, 0, 65, 0, 5, 0, 64, 1, 0, 0, 44, 2, 0, 0, 123, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 45, 2, 0, 0, 44, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 46, 2, 0, 0, 22, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 47, 2, 0, 0, 26, 2, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 48, 2, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 46, 2, 0, 0, 47, 2, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 49, 2, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 45, 2, 0, 0, 48, 2, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 50, 2, 0, 0, 49, 2, 0, 0, 49, 2, 0, 0, 49, 2, 0, 0, 49, 2, 0, 0, 12, 0, 8, 0, 27, 0, 0, 0, 51, 2, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 42, 2, 0, 0, 43, 2, 0, 0, 50, 2, 0, 0, 62, 0, 3, 0, 41, 2, 0, 0, 51, 2, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 52, 2, 0, 0, 6, 2, 0, 0, 186, 0, 5, 0, 84, 0, 0, 0, 53, 2, 0, 0, 52, 2, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 2, 0, 0, 22, 2, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 55, 2, 0, 0, 54, 2, 0, 0, 33, 1, 0, 0, 167, 0, 5, 0, 84, 0, 0, 0, 56, 2, 0, 0, 53, 2, 0, 0, 55, 2, 0, 0, 247, 0, 3, 0, 59, 2, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 56, 2, 0, 0, 58, 2, 0, 0, 64, 2, 0, 0, 248, 0, 2, 0, 58, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 60, 2, 0, 0, 123, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 2, 0, 0, 22, 2, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 62, 2, 0, 0, 61, 2, 0, 0, 61, 2, 0, 0, 61, 2, 0, 0, 61, 2, 0, 0, 12, 0, 8, 0, 27, 0, 0, 0, 63, 2, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 14, 1, 0, 0, 60, 2, 0, 0, 62, 2, 0, 0, 62, 0, 3, 0, 57, 2, 0, 0, 63, 2, 0, 0, 249, 0, 2, 0, 59, 2, 0, 0, 248, 0, 2, 0, 64, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 65, 2, 0, 0, 41, 2, 0, 0, 62, 0, 3, 0, 57, 2, 0, 0, 65, 2, 0, 0, 249, 0, 2, 0, 59, 2, 0, 0, 248, 0, 2, 0, 59, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 66, 2, 0, 0, 57, 2, 0, 0, 62, 0, 3, 0, 51, 1, 0, 0, 66, 2, 0, 0, 249, 0, 2, 0, 252, 1, 0, 0, 248, 0, 2, 0, 252, 1, 0, 0, 249, 0, 2, 0, 163, 1, 0, 0, 248, 0, 2, 0, 163, 1, 0, 0, 249, 0, 2, 0, 60, 1, 0, 0, 248, 0, 2, 0, 60, 1, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 67, 2, 0, 0, 51, 1, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 68, 2, 0, 0, 67, 2, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 70, 2, 0, 0, 68, 2, 0, 0, 69, 2, 0, 0, 247, 0, 3, 0, 72, 2, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 70, 2, 0, 0, 71, 2, 0, 0, 72, 2, 0, 0, 248, 0, 2, 0, 71, 2, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 72, 2, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 76, 2, 0, 0, 51, 1, 0, 0, 62, 0, 3, 0, 75, 2, 0, 0, 76, 2, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 11, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 12, 0, 0, 0, 248, 0, 2, 0, 14, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 49, 0, 0, 0, 11, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 50, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 49, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 51, 0, 0, 0, 12, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 52, 0, 0, 0, 50, 0, 0, 0, 51, 0, 0, 0, 254, 0, 2, 0, 52, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 16, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 17, 0, 0, 0, 248, 0, 2, 0, 19, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 56, 0, 0, 0, 17, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 16, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 58, 0, 0, 0, 17, 0, 0, 0, 136, 0, 5, 0, 6, 0, 0, 0, 59, 0, 0, 0, 57, 0, 0, 0, 58, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 8, 0, 0, 0, 59, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 61, 0, 0, 0, 56, 0, 0, 0, 60, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 62, 0, 0, 0, 55, 0, 0, 0, 61, 0, 0, 0, 254, 0, 2, 0, 62, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 21, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 22, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 23, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 24, 0, 0, 0, 248, 0, 2, 0, 26, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 75, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 76, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 78, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 90, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 105, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 115, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 68, 0, 0, 0, 21, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 71, 0, 0, 0, 21, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 72, 0, 0, 0, 71, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 73, 0, 0, 0, 1, 0, 0, 0, 25, 0, 0, 0, 69, 0, 0, 0, 72, 0, 0, 0, 62, 0, 3, 0, 75, 0, 0, 0, 73, 0, 0, 0, 62, 0, 3, 0, 76, 0, 0, 0, 74, 0, 0, 0, 57, 0, 6, 0, 6, 0, 0, 0, 77, 0, 0, 0, 18, 0, 0, 0, 75, 0, 0, 0, 76, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 77, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 79, 0, 0, 0, 65, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 80, 0, 0, 0, 22, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 81, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 62, 0, 3, 0, 78, 0, 0, 0, 81, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 78, 0, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 85, 0, 0, 0, 82, 0, 0, 0, 83, 0, 0, 0, 247, 0, 3, 0, 87, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 85, 0, 0, 0, 86, 0, 0, 0, 87, 0, 0, 0, 248, 0, 2, 0, 86, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 88, 0, 0, 0, 78, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 89, 0, 0, 0, 88, 0, 0, 0, 74, 0, 0, 0, 62, 0, 3, 0, 78, 0, 0, 0, 89, 0, 0, 0, 249, 0, 2, 0, 87, 0, 0, 0, 248, 0, 2, 0, 87, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 23, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 22, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 91, 0, 0, 0, 92, 0, 0, 0, 62, 0, 3, 0, 90, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 94, 0, 0, 0, 90, 0, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 95, 0, 0, 0, 94, 0, 0, 0, 83, 0, 0, 0, 247, 0, 3, 0, 97, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 95, 0, 0, 0, 96, 0, 0, 0, 97, 0, 0, 0, 248, 0, 2, 0, 96, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 98, 0, 0, 0, 90, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 99, 0, 0, 0, 98, 0, 0, 0, 74, 0, 0, 0, 62, 0, 3, 0, 90, 0, 0, 0, 99, 0, 0, 0, 249, 0, 2, 0, 97, 0, 0, 0, 248, 0, 2, 0, 97, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 100, 0, 0, 0, 78, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 90, 0, 0, 0, 190, 0, 5, 0, 84, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 247, 0, 3, 0, 104, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 102, 0, 0, 0, 103, 0, 0, 0, 104, 0, 0, 0, 248, 0, 2, 0, 103, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 107, 0, 0, 0, 22, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 108, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 107, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 109, 0, 0, 0, 106, 0, 0, 0, 108, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 111, 0, 0, 0, 22, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 112, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 111, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 113, 0, 0, 0, 110, 0, 0, 0, 112, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 114, 0, 0, 0, 109, 0, 0, 0, 113, 0, 0, 0, 62, 0, 3, 0, 105, 0, 0, 0, 114, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 116, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 117, 0, 0, 0, 23, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 14, 0, 0, 0, 117, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 119, 0, 0, 0, 116, 0, 0, 0, 118, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 120, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 121, 0, 0, 0, 23, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 122, 0, 0, 0, 1, 0, 0, 0, 13, 0, 0, 0, 121, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 123, 0, 0, 0, 120, 0, 0, 0, 122, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 124, 0, 0, 0, 119, 0, 0, 0, 123, 0, 0, 0, 62, 0, 3, 0, 115, 0, 0, 0, 124, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 125, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 126, 0, 0, 0, 105, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 127, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 128, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 127, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 129, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 130, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 131, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 132, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 131, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 133, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 128, 0, 0, 0, 132, 0, 0, 0, 254, 0, 2, 0, 133, 0, 0, 0, 248, 0, 2, 0, 104, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 135, 0, 0, 0, 21, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 136, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 135, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 137, 0, 0, 0, 24, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 138, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 139, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 138, 0, 0, 0, 254, 0, 2, 0, 139, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 29, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 30, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 31, 0, 0, 0, 55, 0, 3, 0, 28, 0, 0, 0, 32, 0, 0, 0, 248, 0, 2, 0, 34, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 142, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 146, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 155, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 159, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 168, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 175, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 180, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 185, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 143, 0, 0, 0, 30, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 144, 0, 0, 0, 143, 0, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 145, 0, 0, 0, 83, 0, 0, 0, 144, 0, 0, 0, 247, 0, 3, 0, 148, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 145, 0, 0, 0, 147, 0, 0, 0, 151, 0, 0, 0, 248, 0, 2, 0, 147, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 149, 0, 0, 0, 32, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 150, 0, 0, 0, 149, 0, 0, 0, 149, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 150, 0, 0, 0, 249, 0, 2, 0, 148, 0, 0, 0, 248, 0, 2, 0, 151, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 152, 0, 0, 0, 32, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 153, 0, 0, 0, 152, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 62, 0, 3, 0, 146, 0, 0, 0, 153, 0, 0, 0, 249, 0, 2, 0, 148, 0, 0, 0, 248, 0, 2, 0, 148, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 154, 0, 0, 0, 146, 0, 0, 0, 62, 0, 3, 0, 142, 0, 0, 0, 154, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 156, 0, 0, 0, 30, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 157, 0, 0, 0, 156, 0, 0, 0, 184, 0, 5, 0, 84, 0, 0, 0, 158, 0, 0, 0, 83, 0, 0, 0, 157, 0, 0, 0, 247, 0, 3, 0, 161, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 158, 0, 0, 0, 160, 0, 0, 0, 164, 0, 0, 0, 248, 0, 2, 0, 160, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 162, 0, 0, 0, 142, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 163, 0, 0, 0, 162, 0, 0, 0, 62, 0, 3, 0, 159, 0, 0, 0, 163, 0, 0, 0, 249, 0, 2, 0, 161, 0, 0, 0, 248, 0, 2, 0, 164, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 165, 0, 0, 0, 142, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 166, 0, 0, 0, 165, 0, 0, 0, 62, 0, 3, 0, 159, 0, 0, 0, 166, 0, 0, 0, 249, 0, 2, 0, 161, 0, 0, 0, 248, 0, 2, 0, 161, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 167, 0, 0, 0, 159, 0, 0, 0, 62, 0, 3, 0, 155, 0, 0, 0, 167, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 169, 0, 0, 0, 30, 0, 0, 0, 12, 0, 6, 0, 7, 0, 0, 0, 170, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 169, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 172, 0, 0, 0, 31, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 173, 0, 0, 0, 172, 0, 0, 0, 171, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 174, 0, 0, 0, 170, 0, 0, 0, 173, 0, 0, 0, 62, 0, 3, 0, 168, 0, 0, 0, 174, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 176, 0, 0, 0, 168, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 177, 0, 0, 0, 155, 0, 0, 0, 80, 0, 5, 0, 7, 0, 0, 0, 178, 0, 0, 0, 177, 0, 0, 0, 177, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 179, 0, 0, 0, 176, 0, 0, 0, 178, 0, 0, 0, 62, 0, 3, 0, 175, 0, 0, 0, 179, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 181, 0, 0, 0, 175, 0, 0, 0, 12, 0, 7, 0, 7, 0, 0, 0, 183, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 181, 0, 0, 0, 182, 0, 0, 0, 12, 0, 6, 0, 6, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 66, 0, 0, 0, 183, 0, 0, 0, 62, 0, 3, 0, 180, 0, 0, 0, 184, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 186, 0, 0, 0, 175, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 187, 0, 0, 0, 186, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 188, 0, 0, 0, 175, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 189, 0, 0, 0, 188, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 190, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 187, 0, 0, 0, 189, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 191, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 190, 0, 0, 0, 83, 0, 0, 0, 62, 0, 3, 0, 185, 0, 0, 0, 191, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 192, 0, 0, 0, 180, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 193, 0, 0, 0, 185, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 194, 0, 0, 0, 192, 0, 0, 0, 193, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 195, 0, 0, 0, 155, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 196, 0, 0, 0, 194, 0, 0, 0, 195, 0, 0, 0, 254, 0, 2, 0, 196, 0, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 36, 0, 0, 0, 55, 0, 3, 0, 8, 0, 0, 0, 37, 0, 0, 0, 55, 0, 3, 0, 28, 0, 0, 0, 38, 0, 0, 0, 55, 0, 3, 0, 28, 0, 0, 0, 39, 0, 0, 0, 248, 0, 2, 0, 41, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 199, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 207, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 216, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 220, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 4, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 7, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 19, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 21, 1, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 23, 1, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 200, 0, 0, 0, 37, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 201, 0, 0, 0, 39, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 202, 0, 0, 0, 201, 0, 0, 0, 201, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 203, 0, 0, 0, 200, 0, 0, 0, 202, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 204, 0, 0, 0, 39, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 204, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 206, 0, 0, 0, 203, 0, 0, 0, 205, 0, 0, 0, 62, 0, 3, 0, 199, 0, 0, 0, 206, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 208, 0, 0, 0, 39, 0, 0, 0, 79, 0, 7, 0, 7, 0, 0, 0, 209, 0, 0, 0, 208, 0, 0, 0, 208, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 210, 0, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 211, 0, 0, 0, 210, 0, 0, 0, 171, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 212, 0, 0, 0, 209, 0, 0, 0, 211, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 213, 0, 0, 0, 37, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 214, 0, 0, 0, 213, 0, 0, 0, 171, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 215, 0, 0, 0, 212, 0, 0, 0, 214, 0, 0, 0, 62, 0, 3, 0, 207, 0, 0, 0, 215, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 217, 0, 0, 0, 36, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 218, 0, 0, 0, 207, 0, 0, 0, 131, 0, 5, 0, 7, 0, 0, 0, 219, 0, 0, 0, 217, 0, 0, 0, 218, 0, 0, 0, 62, 0, 3, 0, 216, 0, 0, 0, 219, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 221, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 220, 0, 0, 0, 221, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 222, 0, 0, 0, 220, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 223, 0, 0, 0, 222, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 224, 0, 0, 0, 39, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 225, 0, 0, 0, 224, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 226, 0, 0, 0, 39, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 227, 0, 0, 0, 226, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 228, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 225, 0, 0, 0, 227, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 229, 0, 0, 0, 223, 0, 0, 0, 228, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 230, 0, 0, 0, 220, 0, 0, 0, 70, 0, 0, 0, 62, 0, 3, 0, 230, 0, 0, 0, 229, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 231, 0, 0, 0, 220, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 232, 0, 0, 0, 231, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 234, 0, 0, 0, 39, 0, 0, 0, 233, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 236, 0, 0, 0, 39, 0, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 237, 0, 0, 0, 236, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 238, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 235, 0, 0, 0, 237, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 239, 0, 0, 0, 232, 0, 0, 0, 238, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 240, 0, 0, 0, 220, 0, 0, 0, 67, 0, 0, 0, 62, 0, 3, 0, 240, 0, 0, 0, 239, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 241, 0, 0, 0, 220, 0, 0, 0, 233, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 242, 0, 0, 0, 241, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 243, 0, 0, 0, 39, 0, 0, 0, 233, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 244, 0, 0, 0, 243, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 246, 0, 0, 0, 39, 0, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 247, 0, 0, 0, 246, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 248, 0, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 244, 0, 0, 0, 247, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 249, 0, 0, 0, 242, 0, 0, 0, 248, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 250, 0, 0, 0, 220, 0, 0, 0, 233, 0, 0, 0, 62, 0, 3, 0, 250, 0, 0, 0, 249, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 251, 0, 0, 0, 220, 0, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 252, 0, 0, 0, 251, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 253, 0, 0, 0, 39, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 254, 0, 0, 0, 253, 0, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 255, 0, 0, 0, 39, 0, 0, 0, 245, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 0, 1, 0, 0, 255, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 254, 0, 0, 0, 0, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 2, 1, 0, 0, 252, 0, 0, 0, 1, 1, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 3, 1, 0, 0, 220, 0, 0, 0, 245, 0, 0, 0, 62, 0, 3, 0, 3, 1, 0, 0, 2, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 5, 1, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 6, 1, 0, 0, 5, 1, 0, 0, 171, 0, 0, 0, 62, 0, 3, 0, 4, 1, 0, 0, 6, 1, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 8, 1, 0, 0, 4, 1, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 9, 1, 0, 0, 8, 1, 0, 0, 65, 0, 5, 0, 9, 0, 0, 0, 10, 1, 0, 0, 4, 1, 0, 0, 67, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 12, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 9, 1, 0, 0, 11, 1, 0, 0, 62, 0, 3, 0, 7, 1, 0, 0, 12, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 13, 1, 0, 0, 220, 0, 0, 0, 12, 0, 7, 0, 27, 0, 0, 0, 15, 1, 0, 0, 1, 0, 0, 0, 40, 0, 0, 0, 13, 1, 0, 0, 14, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 16, 1, 0, 0, 7, 1, 0, 0, 80, 0, 7, 0, 27, 0, 0, 0, 17, 1, 0, 0, 16, 1, 0, 0, 16, 1, 0, 0, 16, 1, 0, 0, 16, 1, 0, 0, 12, 0, 7, 0, 27, 0, 0, 0, 18, 1, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 15, 1, 0, 0, 17, 1, 0, 0, 62, 0, 3, 0, 220, 0, 0, 0, 18, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 20, 1, 0, 0, 216, 0, 0, 0, 62, 0, 3, 0, 19, 1, 0, 0, 20, 1, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 22, 1, 0, 0, 199, 0, 0, 0, 62, 0, 3, 0, 21, 1, 0, 0, 22, 1, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 24, 1, 0, 0, 220, 0, 0, 0, 62, 0, 3, 0, 23, 1, 0, 0, 24, 1, 0, 0, 57, 0, 7, 0, 6, 0, 0, 0, 25, 1, 0, 0, 33, 0, 0, 0, 19, 1, 0, 0, 21, 1, 0, 0, 23, 1, 0, 0, 254, 0, 2, 0, 25, 1, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 43, 0, 0, 0, 248, 0, 2, 0, 45, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 28, 1, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 29, 1, 0, 0, 43, 0, 0, 0, 209, 0, 4, 0, 6, 0, 0, 0, 30, 1, 0, 0, 29, 1, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 32, 1, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 30, 1, 0, 0, 31, 1, 0, 0, 62, 0, 3, 0, 28, 1, 0, 0, 32, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 34, 1, 0, 0, 28, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 35, 1, 0, 0, 43, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 36, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 83, 0, 0, 0, 34, 1, 0, 0, 35, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 37, 1, 0, 0, 33, 1, 0, 0, 36, 1, 0, 0, 254, 0, 2, 0, 37, 1, 0, 0, 56, 0, 1, 0, 54, 0, 5, 0, 6, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 55, 0, 3, 0, 9, 0, 0, 0, 46, 0, 0, 0, 248, 0, 2, 0, 48, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 40, 1, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 41, 1, 0, 0, 46, 0, 0, 0, 209, 0, 4, 0, 6, 0, 0, 0, 42, 1, 0, 0, 41, 1, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 44, 1, 0, 0, 42, 1, 0, 0, 43, 1, 0, 0, 62, 0, 3, 0, 40, 1, 0, 0, 44, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 45, 1, 0, 0, 40, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 46, 1, 0, 0, 46, 0, 0, 0, 12, 0, 8, 0, 6, 0, 0, 0, 47, 1, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 83, 0, 0, 0, 45, 1, 0, 0, 46, 1, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 48, 1, 0, 0, 33, 1, 0, 0, 47, 1, 0, 0, 254, 0, 2, 0, 48, 1, 0, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SHAPE_VERT[4908] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 169, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 24, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 18, 0, 0, 0, 53, 0, 0, 0, 56, 0, 0, 0, 101, 0, 0, 0, 123, 0, 0, 0, 124, 0, 0, 0, 126, 0, 0, 0, 134, 0, 0, 0, 136, 0, 0, 0, 138, 0, 0, 0, 140, 0, 0, 0, 141, 0, 0, 0, 144, 0, 0, 0, 145, 0, 0, 0, 147, 0, 0, 0, 148, 0, 0, 0, 151, 0, 0, 0, 152, 0, 0, 0, 157, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 10, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 18, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 51, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 53, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 56, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 99, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 101, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 3, 0, 105, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 110, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 110, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 110, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 8, 0, 110, 0, 0, 0, 2, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 110, 0, 0, 0, 3, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 110, 0, 0, 0, 4, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 112, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 123, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 124, 0, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 126, 0, 0, 0, 118, 95, 112, 111, 105, 110, 116, 0, 5, 0, 4, 0, 134, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 136, 0, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 138, 0, 0, 0, 118, 95, 115, 105, 122, 101, 0, 0, 5, 0, 6, 0, 140, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 6, 0, 141, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 99, 111, 108, 111, 114, 0, 0, 5, 0, 7, 0, 144, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 7, 0, 145, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 0, 5, 0, 6, 0, 147, 0, 0, 0, 118, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 6, 0, 148, 0, 0, 0, 105, 95, 98, 111, 114, 100, 101, 114, 95, 114, 97, 100, 105, 117, 115, 0, 5, 0, 4, 0, 151, 0, 0, 0, 118, 95, 115, 104, 97, 112, 101, 0, 5, 0, 4, 0, 152, 0, 0, 0, 105, 95, 115, 104, 97, 112, 101, 0, 5, 0, 6, 0, 155, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 155, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 155, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 155, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 155, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 157, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 53, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 56, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 101, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 3, 0, 110, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 110, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 5, 0, 110, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 200, 0, 0, 0, 71, 0, 4, 0, 112, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 112, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 123, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 124, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 126, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 134, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 134, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 136, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 138, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 138, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 3, 0, 140, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 140, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 141, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 144, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 144, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 4, 0, 145, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 147, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 147, 0, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 4, 0, 148, 0, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 151, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 151, 0, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 4, 0, 152, 0, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 3, 0, 155, 0, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 155, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 155, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 155, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 155, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 24, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 9, 0, 0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 11, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 13, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 14, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 44, 0, 7, 0, 7, 0, 0, 0, 15, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 23, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 19, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 49, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 50, 0, 0, 0, 7, 0, 0, 0, 49, 0, 0, 0, 32, 0, 4, 0, 52, 0, 0, 0, 1, 0, 0, 0, 49, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 53, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 56, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 59, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 60, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 61, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 62, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 65, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 69, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 59, 0, 0, 0, 76, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 97, 0, 0, 0, 32, 0, 4, 0, 98, 0, 0, 0, 7, 0, 0, 0, 97, 0, 0, 0, 32, 0, 4, 0, 100, 0, 0, 0, 1, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 100, 0, 0, 0, 101, 0, 0, 0, 1, 0, 0, 0, 30, 0, 7, 0, 110, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 49, 0, 0, 0, 49, 0, 0, 0, 32, 0, 4, 0, 111, 0, 0, 0, 2, 0, 0, 0, 110, 0, 0, 0, 59, 0, 4, 0, 111, 0, 0, 0, 112, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 113, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 32, 0, 4, 0, 122, 0, 0, 0, 3, 0, 0, 0, 49, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 123, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 52, 0, 0, 0, 124, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 126, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 63, 32, 0, 4, 0, 133, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 133, 0, 0, 0, 134, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 135, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 135, 0, 0, 0, 136, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 122, 0, 0, 0, 138, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 133, 0, 0, 0, 140, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 135, 0, 0, 0, 141, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 143, 0, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 143, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 21, 0, 0, 0, 145, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 133, 0, 0, 0, 147, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 135, 0, 0, 0, 148, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 150, 0, 0, 0, 3, 0, 0, 0, 19, 0, 0, 0, 59, 0, 4, 0, 150, 0, 0, 0, 151, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 100, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 154, 0, 0, 0, 6, 0, 0, 0, 24, 0, 0, 0, 30, 0, 6, 0, 155, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 154, 0, 0, 0, 154, 0, 0, 0, 32, 0, 4, 0, 156, 0, 0, 0, 3, 0, 0, 0, 155, 0, 0, 0, 59, 0, 4, 0, 156, 0, 0, 0, 157, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 19, 0, 0, 0, 165, 0, 0, 0, 2, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 10, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 50, 0, 0, 0, 51, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 98, 0, 0, 0, 99, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 105, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 9, 0, 0, 0, 107, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 22, 0, 0, 0, 18, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 25, 0, 0, 0, 18, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 27, 0, 0, 0, 23, 0, 0, 0, 26, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 28, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 29, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 30, 0, 0, 0, 13, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 31, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 32, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 34, 0, 0, 0, 14, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 35, 0, 0, 0, 14, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 36, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 37, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 38, 0, 0, 0, 15, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 15, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 40, 0, 0, 0, 27, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 41, 0, 0, 0, 27, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 42, 0, 0, 0, 27, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 43, 0, 0, 0, 27, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 44, 0, 0, 0, 28, 0, 0, 0, 29, 0, 0, 0, 30, 0, 0, 0, 31, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 45, 0, 0, 0, 32, 0, 0, 0, 33, 0, 0, 0, 34, 0, 0, 0, 35, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 46, 0, 0, 0, 36, 0, 0, 0, 37, 0, 0, 0, 38, 0, 0, 0, 39, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 47, 0, 0, 0, 40, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 43, 0, 0, 0, 80, 0, 7, 0, 8, 0, 0, 0, 48, 0, 0, 0, 44, 0, 0, 0, 45, 0, 0, 0, 46, 0, 0, 0, 47, 0, 0, 0, 62, 0, 3, 0, 10, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 127, 0, 4, 0, 49, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 49, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 51, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 63, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 65, 0, 5, 0, 65, 0, 0, 0, 66, 0, 0, 0, 51, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 68, 0, 0, 0, 64, 0, 0, 0, 67, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 70, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 71, 0, 0, 0, 70, 0, 0, 0, 65, 0, 5, 0, 65, 0, 0, 0, 72, 0, 0, 0, 51, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 73, 0, 0, 0, 72, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 74, 0, 0, 0, 71, 0, 0, 0, 73, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 75, 0, 0, 0, 68, 0, 0, 0, 74, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 77, 0, 0, 0, 10, 0, 0, 0, 76, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 79, 0, 0, 0, 78, 0, 0, 0, 12, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 80, 0, 0, 0, 75, 0, 0, 0, 79, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 81, 0, 0, 0, 10, 0, 0, 0, 60, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 82, 0, 0, 0, 81, 0, 0, 0, 129, 0, 5, 0, 7, 0, 0, 0, 83, 0, 0, 0, 80, 0, 0, 0, 82, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 84, 0, 0, 0, 10, 0, 0, 0, 60, 0, 0, 0, 62, 0, 3, 0, 84, 0, 0, 0, 83, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 85, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 86, 0, 0, 0, 85, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 87, 0, 0, 0, 56, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 88, 0, 0, 0, 87, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 89, 0, 0, 0, 86, 0, 0, 0, 88, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 90, 0, 0, 0, 10, 0, 0, 0, 61, 0, 0, 0, 62, 0, 3, 0, 90, 0, 0, 0, 89, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 91, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 92, 0, 0, 0, 91, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 93, 0, 0, 0, 56, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 94, 0, 0, 0, 93, 0, 0, 0, 142, 0, 5, 0, 7, 0, 0, 0, 95, 0, 0, 0, 92, 0, 0, 0, 94, 0, 0, 0, 65, 0, 5, 0, 62, 0, 0, 0, 96, 0, 0, 0, 10, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 96, 0, 0, 0, 95, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 102, 0, 0, 0, 101, 0, 0, 0, 199, 0, 5, 0, 19, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 24, 0, 0, 0, 170, 0, 5, 0, 97, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 24, 0, 0, 0, 62, 0, 3, 0, 99, 0, 0, 0, 104, 0, 0, 0, 61, 0, 4, 0, 97, 0, 0, 0, 106, 0, 0, 0, 99, 0, 0, 0, 247, 0, 3, 0, 109, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 106, 0, 0, 0, 108, 0, 0, 0, 116, 0, 0, 0, 248, 0, 2, 0, 108, 0, 0, 0, 65, 0, 5, 0, 113, 0, 0, 0, 114, 0, 0, 0, 112, 0, 0, 0, 61, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 62, 0, 3, 0, 107, 0, 0, 0, 115, 0, 0, 0, 249, 0, 2, 0, 109, 0, 0, 0, 248, 0, 2, 0, 116, 0, 0, 0, 65, 0, 5, 0, 113, 0, 0, 0, 117, 0, 0, 0, 112, 0, 0, 0, 69, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 118, 0, 0, 0, 117, 0, 0, 0, 62, 0, 3, 0, 107, 0, 0, 0, 118, 0, 0, 0, 249, 0, 2, 0, 109, 0, 0, 0, 248, 0, 2, 0, 109, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 119, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 120, 0, 0, 0, 10, 0, 0, 0, 146, 0, 5, 0, 8, 0, 0, 0, 121, 0, 0, 0, 119, 0, 0, 0, 120, 0, 0, 0, 62, 0, 3, 0, 105, 0, 0, 0, 121, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 125, 0, 0, 0, 124, 0, 0, 0, 62, 0, 3, 0, 123, 0, 0, 0, 125, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 127, 0, 0, 0, 124, 0, 0, 0, 80, 0, 5, 0, 49, 0, 0, 0, 129, 0, 0, 0, 128, 0, 0, 0, 128, 0, 0, 0, 131, 0, 5, 0, 49, 0, 0, 0, 130, 0, 0, 0, 127, 0, 0, 0, 129, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 131, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 49, 0, 0, 0, 132, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 62, 0, 3, 0, 126, 0, 0, 0, 132, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 137, 0, 0, 0, 136, 0, 0, 0, 62, 0, 3, 0, 134, 0, 0, 0, 137, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 139, 0, 0, 0, 56, 0, 0, 0, 62, 0, 3, 0, 138, 0, 0, 0, 139, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 142, 0, 0, 0, 141, 0, 0, 0, 62, 0, 3, 0, 140, 0, 0, 0, 142, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 146, 0, 0, 0, 145, 0, 0, 0, 62, 0, 3, 0, 144, 0, 0, 0, 146, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 62, 0, 3, 0, 147, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 19, 0, 0, 0, 153, 0, 0, 0, 152, 0, 0, 0, 62, 0, 3, 0, 151, 0, 0, 0, 153, 0, 0, 0, 61, 0, 4, 0, 8, 0, 0, 0, 158, 0, 0, 0, 105, 0, 0, 0, 61, 0, 4, 0, 49, 0, 0, 0, 159, 0, 0, 0, 124, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 159, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 161, 0, 0, 0, 159, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 162, 0, 0, 0, 160, 0, 0, 0, 161, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 145, 0, 5, 0, 7, 0, 0, 0, 163, 0, 0, 0, 158, 0, 0, 0, 162, 0, 0, 0, 65, 0, 5, 0, 133, 0, 0, 0, 164, 0, 0, 0, 157, 0, 0, 0, 61, 0, 0, 0, 62, 0, 3, 0, 164, 0, 0, 0, 163, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 166, 0, 0, 0, 18, 0, 0, 0, 165, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 167, 0, 0, 0, 166, 0, 0, 0, 65, 0, 6, 0, 143, 0, 0, 0, 168, 0, 0, 0, 157, 0, 0, 0, 61, 0, 0, 0, 165, 0, 0, 0, 62, 0, 3, 0, 168, 0, 0, 0, 167, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SPRITE_FRAG[3524] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 163, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 10, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 14, 0, 0, 0, 35, 0, 0, 0, 140, 0, 0, 0, 161, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 4, 0, 9, 0, 0, 0, 99, 111, 108, 111, 114, 0, 0, 0, 5, 0, 4, 0, 11, 0, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 14, 0, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 4, 0, 22, 0, 0, 0, 111, 117, 116, 108, 105, 110, 101, 0, 5, 0, 5, 0, 25, 0, 0, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 5, 0, 5, 0, 29, 0, 0, 0, 117, 95, 115, 97, 109, 112, 108, 101, 114, 0, 0, 0, 5, 0, 4, 0, 35, 0, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 3, 0, 133, 0, 0, 0, 99, 0, 0, 0, 5, 0, 6, 0, 140, 0, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 5, 0, 161, 0, 0, 0, 102, 114, 97, 103, 95, 99, 111, 108, 111, 114, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 3, 0, 14, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 14, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 25, 0, 0, 0, 33, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 25, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 29, 0, 0, 0, 33, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 29, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 35, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 3, 0, 140, 0, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 140, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 161, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 13, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 13, 0, 0, 0, 14, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 20, 0, 2, 0, 17, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 25, 0, 9, 0, 23, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 24, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 59, 0, 4, 0, 24, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 26, 0, 2, 0, 27, 0, 0, 0, 32, 0, 4, 0, 28, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 59, 0, 4, 0, 28, 0, 0, 0, 29, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 31, 0, 0, 0, 23, 0, 0, 0, 23, 0, 4, 0, 33, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 34, 0, 0, 0, 1, 0, 0, 0, 33, 0, 0, 0, 59, 0, 4, 0, 34, 0, 0, 0, 35, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 41, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 41, 0, 0, 0, 42, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 131, 0, 0, 0, 0, 0, 128, 63, 59, 0, 4, 0, 10, 0, 0, 0, 140, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 155, 0, 0, 0, 205, 204, 204, 60, 32, 0, 4, 0, 160, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 160, 0, 0, 0, 161, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 21, 0, 0, 0, 22, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 133, 0, 0, 0, 7, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 15, 0, 0, 0, 14, 0, 0, 0, 186, 0, 5, 0, 17, 0, 0, 0, 18, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 247, 0, 3, 0, 20, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 18, 0, 0, 0, 19, 0, 0, 0, 145, 0, 0, 0, 248, 0, 2, 0, 19, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 26, 0, 0, 0, 30, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 37, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 16, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 40, 0, 0, 0, 32, 0, 0, 0, 39, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 43, 0, 0, 0, 40, 0, 0, 0, 3, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 43, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 44, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 45, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 46, 0, 0, 0, 44, 0, 0, 0, 45, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 47, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 49, 0, 0, 0, 48, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 16, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 51, 0, 0, 0, 47, 0, 0, 0, 50, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 52, 0, 0, 0, 46, 0, 0, 0, 51, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 53, 0, 0, 0, 52, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 54, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 55, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 56, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 57, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 58, 0, 0, 0, 56, 0, 0, 0, 57, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 59, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 60, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 61, 0, 0, 0, 16, 0, 0, 0, 60, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 62, 0, 0, 0, 59, 0, 0, 0, 61, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 63, 0, 0, 0, 58, 0, 0, 0, 62, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 63, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 65, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 66, 0, 0, 0, 65, 0, 0, 0, 64, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 66, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 67, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 68, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 69, 0, 0, 0, 67, 0, 0, 0, 68, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 70, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 71, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 72, 0, 0, 0, 71, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 73, 0, 0, 0, 16, 0, 0, 0, 72, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 74, 0, 0, 0, 70, 0, 0, 0, 73, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 75, 0, 0, 0, 69, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 76, 0, 0, 0, 75, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 77, 0, 0, 0, 76, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 78, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 79, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 80, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 81, 0, 0, 0, 79, 0, 0, 0, 80, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 82, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 83, 0, 0, 0, 14, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 84, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 84, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 86, 0, 0, 0, 83, 0, 0, 0, 85, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 87, 0, 0, 0, 82, 0, 0, 0, 86, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 88, 0, 0, 0, 81, 0, 0, 0, 87, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 89, 0, 0, 0, 88, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 90, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 91, 0, 0, 0, 90, 0, 0, 0, 89, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 91, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 92, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 93, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 94, 0, 0, 0, 92, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 95, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 97, 0, 0, 0, 96, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 98, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 99, 0, 0, 0, 97, 0, 0, 0, 98, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 100, 0, 0, 0, 95, 0, 0, 0, 99, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 101, 0, 0, 0, 94, 0, 0, 0, 100, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 101, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 103, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 104, 0, 0, 0, 103, 0, 0, 0, 102, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 104, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 105, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 106, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 108, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 14, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 110, 0, 0, 0, 109, 0, 0, 0, 109, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 111, 0, 0, 0, 108, 0, 0, 0, 110, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 112, 0, 0, 0, 107, 0, 0, 0, 111, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 113, 0, 0, 0, 112, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 114, 0, 0, 0, 113, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 115, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 116, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 117, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 118, 0, 0, 0, 116, 0, 0, 0, 117, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 119, 0, 0, 0, 35, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 120, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 121, 0, 0, 0, 120, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 122, 0, 0, 0, 14, 0, 0, 0, 127, 0, 4, 0, 6, 0, 0, 0, 123, 0, 0, 0, 122, 0, 0, 0, 80, 0, 5, 0, 33, 0, 0, 0, 124, 0, 0, 0, 121, 0, 0, 0, 123, 0, 0, 0, 129, 0, 5, 0, 33, 0, 0, 0, 125, 0, 0, 0, 119, 0, 0, 0, 124, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 126, 0, 0, 0, 118, 0, 0, 0, 125, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 126, 0, 0, 0, 3, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 128, 0, 0, 0, 22, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 128, 0, 0, 0, 127, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 129, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 130, 0, 0, 0, 22, 0, 0, 0, 12, 0, 7, 0, 6, 0, 0, 0, 132, 0, 0, 0, 1, 0, 0, 0, 37, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 62, 0, 3, 0, 22, 0, 0, 0, 132, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 134, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 135, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 136, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 137, 0, 0, 0, 35, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 138, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 62, 0, 3, 0, 133, 0, 0, 0, 138, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 139, 0, 0, 0, 133, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 141, 0, 0, 0, 140, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 142, 0, 0, 0, 22, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 143, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 142, 0, 0, 0, 12, 0, 8, 0, 7, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 46, 0, 0, 0, 139, 0, 0, 0, 141, 0, 0, 0, 143, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 144, 0, 0, 0, 249, 0, 2, 0, 20, 0, 0, 0, 248, 0, 2, 0, 145, 0, 0, 0, 61, 0, 4, 0, 23, 0, 0, 0, 146, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 27, 0, 0, 0, 147, 0, 0, 0, 29, 0, 0, 0, 86, 0, 5, 0, 31, 0, 0, 0, 148, 0, 0, 0, 146, 0, 0, 0, 147, 0, 0, 0, 61, 0, 4, 0, 33, 0, 0, 0, 149, 0, 0, 0, 35, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 150, 0, 0, 0, 148, 0, 0, 0, 149, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 151, 0, 0, 0, 11, 0, 0, 0, 133, 0, 5, 0, 7, 0, 0, 0, 152, 0, 0, 0, 150, 0, 0, 0, 151, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 152, 0, 0, 0, 249, 0, 2, 0, 20, 0, 0, 0, 248, 0, 2, 0, 20, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 153, 0, 0, 0, 9, 0, 0, 0, 42, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 154, 0, 0, 0, 153, 0, 0, 0, 184, 0, 5, 0, 17, 0, 0, 0, 156, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 247, 0, 3, 0, 158, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 156, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 248, 0, 2, 0, 157, 0, 0, 0, 252, 0, 1, 0, 248, 0, 2, 0, 158, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 162, 0, 0, 0, 9, 0, 0, 0, 62, 0, 3, 0, 161, 0, 0, 0, 162, 0, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

static const unsigned char VULKAN_SPRITE_VERT[7752] = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 8, 0, 41, 1, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 20, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 11, 0, 0, 0, 147, 0, 0, 0, 181, 0, 0, 0, 184, 0, 0, 0, 228, 0, 0, 0, 6, 1, 0, 0, 7, 1, 0, 0, 9, 1, 0, 0, 17, 1, 0, 0, 18, 1, 0, 0, 20, 1, 0, 0, 21, 1, 0, 0, 24, 1, 0, 0, 25, 1, 0, 0, 30, 1, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, 194, 1, 0, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 3, 0, 8, 0, 0, 0, 113, 120, 120, 0, 5, 0, 5, 0, 11, 0, 0, 0, 105, 95, 114, 111, 116, 97, 116, 105, 111, 110, 0, 0, 5, 0, 3, 0, 20, 0, 0, 0, 113, 121, 121, 0, 5, 0, 3, 0, 27, 0, 0, 0, 113, 122, 122, 0, 5, 0, 3, 0, 34, 0, 0, 0, 113, 120, 122, 0, 5, 0, 3, 0, 40, 0, 0, 0, 113, 120, 121, 0, 5, 0, 3, 0, 46, 0, 0, 0, 113, 121, 122, 0, 5, 0, 3, 0, 52, 0, 0, 0, 113, 119, 120, 0, 5, 0, 3, 0, 59, 0, 0, 0, 113, 119, 121, 0, 5, 0, 3, 0, 65, 0, 0, 0, 113, 119, 122, 0, 5, 0, 6, 0, 73, 0, 0, 0, 114, 111, 116, 97, 116, 105, 111, 110, 95, 109, 97, 116, 114, 105, 120, 0, 5, 0, 5, 0, 141, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 0, 0, 0, 5, 0, 5, 0, 147, 0, 0, 0, 105, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 4, 0, 179, 0, 0, 0, 111, 102, 102, 115, 101, 116, 0, 0, 5, 0, 5, 0, 181, 0, 0, 0, 105, 95, 111, 102, 102, 115, 101, 116, 0, 0, 0, 0, 5, 0, 4, 0, 184, 0, 0, 0, 105, 95, 115, 105, 122, 101, 0, 0, 5, 0, 4, 0, 226, 0, 0, 0, 105, 115, 95, 117, 105, 0, 0, 0, 5, 0, 4, 0, 228, 0, 0, 0, 105, 95, 102, 108, 97, 103, 115, 0, 5, 0, 7, 0, 232, 0, 0, 0, 105, 103, 110, 111, 114, 101, 95, 99, 97, 109, 101, 114, 97, 95, 122, 111, 111, 109, 0, 0, 5, 0, 3, 0, 236, 0, 0, 0, 109, 118, 112, 0, 5, 0, 7, 0, 241, 0, 0, 0, 71, 108, 111, 98, 97, 108, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 0, 6, 0, 8, 0, 241, 0, 0, 0, 0, 0, 0, 0, 115, 99, 114, 101, 101, 110, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 6, 0, 9, 0, 241, 0, 0, 0, 2, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 3, 0, 0, 0, 110, 111, 122, 111, 111, 109, 95, 112, 114, 111, 106, 101, 99, 116, 105, 111, 110, 0, 0, 0, 6, 0, 8, 0, 241, 0, 0, 0, 4, 0, 0, 0, 116, 114, 97, 110, 115, 102, 111, 114, 109, 95, 109, 97, 116, 114, 105, 120, 0, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 5, 0, 0, 0, 105, 110, 118, 95, 118, 105, 101, 119, 95, 112, 114, 111, 106, 0, 0, 0, 6, 0, 7, 0, 241, 0, 0, 0, 6, 0, 0, 0, 99, 97, 109, 101, 114, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 6, 0, 241, 0, 0, 0, 7, 0, 0, 0, 119, 105, 110, 100, 111, 119, 95, 115, 105, 122, 101, 0, 5, 0, 5, 0, 243, 0, 0, 0, 103, 108, 111, 98, 97, 108, 95, 117, 98, 111, 0, 0, 5, 0, 4, 0, 6, 1, 0, 0, 118, 95, 117, 118, 0, 0, 0, 0, 5, 0, 5, 0, 7, 1, 0, 0, 97, 95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 7, 0, 9, 1, 0, 0, 105, 95, 117, 118, 95, 111, 102, 102, 115, 101, 116, 95, 115, 99, 97, 108, 101, 0, 0, 0, 5, 0, 4, 0, 17, 1, 0, 0, 118, 95, 99, 111, 108, 111, 114, 0, 5, 0, 4, 0, 18, 1, 0, 0, 105, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 20, 1, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 6, 0, 21, 1, 0, 0, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 99, 111, 108, 111, 114, 0, 5, 0, 7, 0, 24, 1, 0, 0, 118, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 7, 0, 25, 1, 0, 0, 105, 95, 111, 117, 116, 108, 105, 110, 101, 95, 116, 104, 105, 99, 107, 110, 101, 115, 115, 0, 5, 0, 6, 0, 28, 1, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 28, 1, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 28, 1, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 28, 1, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 28, 1, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 30, 1, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 11, 0, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 147, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 181, 0, 0, 0, 30, 0, 0, 0, 4, 0, 0, 0, 71, 0, 4, 0, 184, 0, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 228, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 71, 0, 3, 0, 241, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, 128, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 192, 0, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 4, 0, 0, 0, 35, 0, 0, 0, 0, 1, 0, 0, 72, 0, 4, 0, 241, 0, 0, 0, 5, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 5, 0, 0, 0, 35, 0, 0, 0, 64, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 6, 0, 0, 0, 35, 0, 0, 0, 128, 1, 0, 0, 72, 0, 5, 0, 241, 0, 0, 0, 7, 0, 0, 0, 35, 0, 0, 0, 136, 1, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 33, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 243, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 6, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 7, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 9, 1, 0, 0, 30, 0, 0, 0, 5, 0, 0, 0, 71, 0, 3, 0, 17, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 17, 1, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 18, 1, 0, 0, 30, 0, 0, 0, 6, 0, 0, 0, 71, 0, 3, 0, 20, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 20, 1, 0, 0, 30, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 21, 1, 0, 0, 30, 0, 0, 0, 7, 0, 0, 0, 71, 0, 3, 0, 24, 1, 0, 0, 14, 0, 0, 0, 71, 0, 4, 0, 24, 1, 0, 0, 30, 0, 0, 0, 3, 0, 0, 0, 71, 0, 4, 0, 25, 1, 0, 0, 30, 0, 0, 0, 8, 0, 0, 0, 71, 0, 3, 0, 28, 1, 0, 0, 2, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 28, 1, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 32, 0, 4, 0, 7, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 23, 0, 4, 0, 9, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 10, 0, 0, 0, 1, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 12, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 6, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 28, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 12, 0, 0, 0, 53, 0, 0, 0, 3, 0, 0, 0, 24, 0, 4, 0, 71, 0, 0, 0, 9, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 72, 0, 0, 0, 7, 0, 0, 0, 71, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 74, 0, 0, 0, 0, 0, 128, 63, 43, 0, 4, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 64, 43, 0, 4, 0, 6, 0, 0, 0, 89, 0, 0, 0, 0, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 119, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 142, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 143, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 44, 0, 7, 0, 9, 0, 0, 0, 144, 0, 0, 0, 89, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 89, 0, 0, 0, 23, 0, 4, 0, 145, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 146, 0, 0, 0, 1, 0, 0, 0, 145, 0, 0, 0, 59, 0, 4, 0, 146, 0, 0, 0, 147, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 177, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 178, 0, 0, 0, 7, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 180, 0, 0, 0, 1, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 181, 0, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 184, 0, 0, 0, 1, 0, 0, 0, 21, 0, 4, 0, 187, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 188, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 32, 0, 4, 0, 190, 0, 0, 0, 7, 0, 0, 0, 9, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 196, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 187, 0, 0, 0, 203, 0, 0, 0, 2, 0, 0, 0, 20, 0, 2, 0, 224, 0, 0, 0, 32, 0, 4, 0, 225, 0, 0, 0, 7, 0, 0, 0, 224, 0, 0, 0, 32, 0, 4, 0, 227, 0, 0, 0, 1, 0, 0, 0, 12, 0, 0, 0, 59, 0, 4, 0, 227, 0, 0, 0, 228, 0, 0, 0, 1, 0, 0, 0, 30, 0, 10, 0, 241, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 71, 0, 0, 0, 177, 0, 0, 0, 177, 0, 0, 0, 32, 0, 4, 0, 242, 0, 0, 0, 2, 0, 0, 0, 241, 0, 0, 0, 59, 0, 4, 0, 242, 0, 0, 0, 243, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 244, 0, 0, 0, 2, 0, 0, 0, 71, 0, 0, 0, 32, 0, 4, 0, 5, 1, 0, 0, 3, 0, 0, 0, 177, 0, 0, 0, 59, 0, 4, 0, 5, 1, 0, 0, 6, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 180, 0, 0, 0, 7, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 9, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 16, 1, 0, 0, 3, 0, 0, 0, 9, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 17, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 18, 1, 0, 0, 1, 0, 0, 0, 59, 0, 4, 0, 16, 1, 0, 0, 20, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 10, 0, 0, 0, 21, 1, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 23, 1, 0, 0, 3, 0, 0, 0, 6, 0, 0, 0, 59, 0, 4, 0, 23, 1, 0, 0, 24, 1, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 25, 1, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 27, 1, 0, 0, 6, 0, 0, 0, 21, 0, 0, 0, 30, 0, 6, 0, 28, 1, 0, 0, 9, 0, 0, 0, 6, 0, 0, 0, 27, 1, 0, 0, 27, 1, 0, 0, 32, 0, 4, 0, 29, 1, 0, 0, 3, 0, 0, 0, 28, 1, 0, 0, 59, 0, 4, 0, 29, 1, 0, 0, 30, 1, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 248, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 20, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 27, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 34, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 40, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 46, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 52, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 59, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 7, 0, 0, 0, 65, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 73, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 141, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 178, 0, 0, 0, 179, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 226, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 225, 0, 0, 0, 232, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 236, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 238, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 72, 0, 0, 0, 249, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 15, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 17, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 18, 0, 0, 0, 17, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 19, 0, 0, 0, 16, 0, 0, 0, 18, 0, 0, 0, 62, 0, 3, 0, 8, 0, 0, 0, 19, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 22, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 24, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 24, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 25, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 29, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 31, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 32, 0, 0, 0, 31, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 33, 0, 0, 0, 30, 0, 0, 0, 32, 0, 0, 0, 62, 0, 3, 0, 27, 0, 0, 0, 33, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 35, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 36, 0, 0, 0, 35, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 37, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 39, 0, 0, 0, 36, 0, 0, 0, 38, 0, 0, 0, 62, 0, 3, 0, 34, 0, 0, 0, 39, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 41, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 42, 0, 0, 0, 41, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 43, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 44, 0, 0, 0, 43, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 45, 0, 0, 0, 42, 0, 0, 0, 44, 0, 0, 0, 62, 0, 3, 0, 40, 0, 0, 0, 45, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 47, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 48, 0, 0, 0, 47, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 49, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 51, 0, 0, 0, 48, 0, 0, 0, 50, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 51, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 54, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 55, 0, 0, 0, 54, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 56, 0, 0, 0, 11, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 57, 0, 0, 0, 56, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 58, 0, 0, 0, 55, 0, 0, 0, 57, 0, 0, 0, 62, 0, 3, 0, 52, 0, 0, 0, 58, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 60, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 62, 0, 0, 0, 11, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 63, 0, 0, 0, 62, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 63, 0, 0, 0, 62, 0, 3, 0, 59, 0, 0, 0, 64, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 66, 0, 0, 0, 11, 0, 0, 0, 53, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 67, 0, 0, 0, 66, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 68, 0, 0, 0, 11, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 69, 0, 0, 0, 68, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 70, 0, 0, 0, 67, 0, 0, 0, 69, 0, 0, 0, 62, 0, 3, 0, 65, 0, 0, 0, 70, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 76, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 77, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 78, 0, 0, 0, 76, 0, 0, 0, 77, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 79, 0, 0, 0, 75, 0, 0, 0, 78, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 80, 0, 0, 0, 74, 0, 0, 0, 79, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 81, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 82, 0, 0, 0, 65, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 83, 0, 0, 0, 81, 0, 0, 0, 82, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 84, 0, 0, 0, 75, 0, 0, 0, 83, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 85, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 86, 0, 0, 0, 59, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 87, 0, 0, 0, 85, 0, 0, 0, 86, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 88, 0, 0, 0, 75, 0, 0, 0, 87, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 90, 0, 0, 0, 80, 0, 0, 0, 84, 0, 0, 0, 88, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 91, 0, 0, 0, 40, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 92, 0, 0, 0, 65, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 93, 0, 0, 0, 91, 0, 0, 0, 92, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 94, 0, 0, 0, 75, 0, 0, 0, 93, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 95, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 96, 0, 0, 0, 27, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 97, 0, 0, 0, 95, 0, 0, 0, 96, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 98, 0, 0, 0, 75, 0, 0, 0, 97, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 99, 0, 0, 0, 74, 0, 0, 0, 98, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 100, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 101, 0, 0, 0, 52, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 102, 0, 0, 0, 100, 0, 0, 0, 101, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 103, 0, 0, 0, 75, 0, 0, 0, 102, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 104, 0, 0, 0, 94, 0, 0, 0, 99, 0, 0, 0, 103, 0, 0, 0, 89, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 105, 0, 0, 0, 34, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 106, 0, 0, 0, 59, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 107, 0, 0, 0, 105, 0, 0, 0, 106, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 108, 0, 0, 0, 75, 0, 0, 0, 107, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 109, 0, 0, 0, 46, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 110, 0, 0, 0, 52, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 111, 0, 0, 0, 109, 0, 0, 0, 110, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 112, 0, 0, 0, 75, 0, 0, 0, 111, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 113, 0, 0, 0, 8, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 114, 0, 0, 0, 20, 0, 0, 0, 129, 0, 5, 0, 6, 0, 0, 0, 115, 0, 0, 0, 113, 0, 0, 0, 114, 0, 0, 0, 133, 0, 5, 0, 6, 0, 0, 0, 116, 0, 0, 0, 75, 0, 0, 0, 115, 0, 0, 0, 131, 0, 5, 0, 6, 0, 0, 0, 117, 0, 0, 0, 74, 0, 0, 0, 116, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 118, 0, 0, 0, 108, 0, 0, 0, 112, 0, 0, 0, 117, 0, 0, 0, 89, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 120, 0, 0, 0, 90, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 121, 0, 0, 0, 90, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 122, 0, 0, 0, 90, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 123, 0, 0, 0, 90, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 124, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 125, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 126, 0, 0, 0, 104, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 127, 0, 0, 0, 104, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 128, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 129, 0, 0, 0, 118, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 130, 0, 0, 0, 118, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 131, 0, 0, 0, 118, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 132, 0, 0, 0, 119, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 133, 0, 0, 0, 119, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 134, 0, 0, 0, 119, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 135, 0, 0, 0, 119, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 136, 0, 0, 0, 120, 0, 0, 0, 121, 0, 0, 0, 122, 0, 0, 0, 123, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 137, 0, 0, 0, 124, 0, 0, 0, 125, 0, 0, 0, 126, 0, 0, 0, 127, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 138, 0, 0, 0, 128, 0, 0, 0, 129, 0, 0, 0, 130, 0, 0, 0, 131, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 139, 0, 0, 0, 132, 0, 0, 0, 133, 0, 0, 0, 134, 0, 0, 0, 135, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 140, 0, 0, 0, 136, 0, 0, 0, 137, 0, 0, 0, 138, 0, 0, 0, 139, 0, 0, 0, 62, 0, 3, 0, 73, 0, 0, 0, 140, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 148, 0, 0, 0, 147, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 149, 0, 0, 0, 148, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 150, 0, 0, 0, 147, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 151, 0, 0, 0, 150, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 152, 0, 0, 0, 149, 0, 0, 0, 151, 0, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 153, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 154, 0, 0, 0, 142, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 155, 0, 0, 0, 142, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 156, 0, 0, 0, 142, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 157, 0, 0, 0, 143, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 158, 0, 0, 0, 143, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 159, 0, 0, 0, 143, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 160, 0, 0, 0, 143, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 161, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 162, 0, 0, 0, 144, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 163, 0, 0, 0, 144, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 164, 0, 0, 0, 144, 0, 0, 0, 3, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 165, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 166, 0, 0, 0, 152, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 167, 0, 0, 0, 152, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 168, 0, 0, 0, 152, 0, 0, 0, 3, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 169, 0, 0, 0, 153, 0, 0, 0, 154, 0, 0, 0, 155, 0, 0, 0, 156, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 170, 0, 0, 0, 157, 0, 0, 0, 158, 0, 0, 0, 159, 0, 0, 0, 160, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 171, 0, 0, 0, 161, 0, 0, 0, 162, 0, 0, 0, 163, 0, 0, 0, 164, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 172, 0, 0, 0, 165, 0, 0, 0, 166, 0, 0, 0, 167, 0, 0, 0, 168, 0, 0, 0, 80, 0, 7, 0, 71, 0, 0, 0, 173, 0, 0, 0, 169, 0, 0, 0, 170, 0, 0, 0, 171, 0, 0, 0, 172, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 173, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 174, 0, 0, 0, 73, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 175, 0, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 176, 0, 0, 0, 175, 0, 0, 0, 174, 0, 0, 0, 62, 0, 3, 0, 141, 0, 0, 0, 176, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 182, 0, 0, 0, 181, 0, 0, 0, 127, 0, 4, 0, 177, 0, 0, 0, 183, 0, 0, 0, 182, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 185, 0, 0, 0, 184, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 186, 0, 0, 0, 183, 0, 0, 0, 185, 0, 0, 0, 62, 0, 3, 0, 179, 0, 0, 0, 186, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 191, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 192, 0, 0, 0, 191, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 193, 0, 0, 0, 179, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 194, 0, 0, 0, 193, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 195, 0, 0, 0, 192, 0, 0, 0, 194, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 197, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 198, 0, 0, 0, 197, 0, 0, 0, 65, 0, 5, 0, 7, 0, 0, 0, 199, 0, 0, 0, 179, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 200, 0, 0, 0, 199, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 201, 0, 0, 0, 198, 0, 0, 0, 200, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 202, 0, 0, 0, 195, 0, 0, 0, 201, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 204, 0, 0, 0, 141, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 205, 0, 0, 0, 204, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 206, 0, 0, 0, 205, 0, 0, 0, 89, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 207, 0, 0, 0, 202, 0, 0, 0, 206, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 208, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 209, 0, 0, 0, 208, 0, 0, 0, 129, 0, 5, 0, 9, 0, 0, 0, 210, 0, 0, 0, 207, 0, 0, 0, 209, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 211, 0, 0, 0, 141, 0, 0, 0, 188, 0, 0, 0, 62, 0, 3, 0, 211, 0, 0, 0, 210, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 212, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 213, 0, 0, 0, 212, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 214, 0, 0, 0, 184, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 215, 0, 0, 0, 214, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 216, 0, 0, 0, 213, 0, 0, 0, 215, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 217, 0, 0, 0, 141, 0, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 217, 0, 0, 0, 216, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 218, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 219, 0, 0, 0, 218, 0, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 220, 0, 0, 0, 184, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 221, 0, 0, 0, 220, 0, 0, 0, 142, 0, 5, 0, 9, 0, 0, 0, 222, 0, 0, 0, 219, 0, 0, 0, 221, 0, 0, 0, 65, 0, 5, 0, 190, 0, 0, 0, 223, 0, 0, 0, 141, 0, 0, 0, 196, 0, 0, 0, 62, 0, 3, 0, 223, 0, 0, 0, 222, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 229, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 230, 0, 0, 0, 229, 0, 0, 0, 21, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 231, 0, 0, 0, 230, 0, 0, 0, 21, 0, 0, 0, 62, 0, 3, 0, 226, 0, 0, 0, 231, 0, 0, 0, 61, 0, 4, 0, 12, 0, 0, 0, 233, 0, 0, 0, 228, 0, 0, 0, 199, 0, 5, 0, 12, 0, 0, 0, 234, 0, 0, 0, 233, 0, 0, 0, 28, 0, 0, 0, 170, 0, 5, 0, 224, 0, 0, 0, 235, 0, 0, 0, 234, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 232, 0, 0, 0, 235, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 237, 0, 0, 0, 226, 0, 0, 0, 247, 0, 3, 0, 240, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 237, 0, 0, 0, 239, 0, 0, 0, 247, 0, 0, 0, 248, 0, 2, 0, 239, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 245, 0, 0, 0, 243, 0, 0, 0, 189, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 246, 0, 0, 0, 245, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 246, 0, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 247, 0, 0, 0, 61, 0, 4, 0, 224, 0, 0, 0, 248, 0, 0, 0, 232, 0, 0, 0, 247, 0, 3, 0, 251, 0, 0, 0, 0, 0, 0, 0, 250, 0, 4, 0, 248, 0, 0, 0, 250, 0, 0, 0, 254, 0, 0, 0, 248, 0, 2, 0, 250, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 252, 0, 0, 0, 243, 0, 0, 0, 203, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 253, 0, 0, 0, 252, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 253, 0, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 254, 0, 0, 0, 65, 0, 5, 0, 244, 0, 0, 0, 255, 0, 0, 0, 243, 0, 0, 0, 196, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 0, 1, 0, 0, 255, 0, 0, 0, 62, 0, 3, 0, 249, 0, 0, 0, 0, 1, 0, 0, 249, 0, 2, 0, 251, 0, 0, 0, 248, 0, 2, 0, 251, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 1, 1, 0, 0, 249, 0, 0, 0, 62, 0, 3, 0, 238, 0, 0, 0, 1, 1, 0, 0, 249, 0, 2, 0, 240, 0, 0, 0, 248, 0, 2, 0, 240, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 2, 1, 0, 0, 238, 0, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 3, 1, 0, 0, 141, 0, 0, 0, 146, 0, 5, 0, 71, 0, 0, 0, 4, 1, 0, 0, 2, 1, 0, 0, 3, 1, 0, 0, 62, 0, 3, 0, 236, 0, 0, 0, 4, 1, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 8, 1, 0, 0, 7, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 10, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 11, 1, 0, 0, 10, 1, 0, 0, 10, 1, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 133, 0, 5, 0, 177, 0, 0, 0, 12, 1, 0, 0, 8, 1, 0, 0, 11, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 13, 1, 0, 0, 9, 1, 0, 0, 79, 0, 7, 0, 177, 0, 0, 0, 14, 1, 0, 0, 13, 1, 0, 0, 13, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 129, 0, 5, 0, 177, 0, 0, 0, 15, 1, 0, 0, 12, 1, 0, 0, 14, 1, 0, 0, 62, 0, 3, 0, 6, 1, 0, 0, 15, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 19, 1, 0, 0, 18, 1, 0, 0, 62, 0, 3, 0, 17, 1, 0, 0, 19, 1, 0, 0, 61, 0, 4, 0, 9, 0, 0, 0, 22, 1, 0, 0, 21, 1, 0, 0, 62, 0, 3, 0, 20, 1, 0, 0, 22, 1, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 26, 1, 0, 0, 25, 1, 0, 0, 62, 0, 3, 0, 24, 1, 0, 0, 26, 1, 0, 0, 61, 0, 4, 0, 71, 0, 0, 0, 31, 1, 0, 0, 236, 0, 0, 0, 61, 0, 4, 0, 177, 0, 0, 0, 32, 1, 0, 0, 7, 1, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 33, 1, 0, 0, 32, 1, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 34, 1, 0, 0, 32, 1, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 9, 0, 0, 0, 35, 1, 0, 0, 33, 1, 0, 0, 34, 1, 0, 0, 89, 0, 0, 0, 74, 0, 0, 0, 145, 0, 5, 0, 9, 0, 0, 0, 36, 1, 0, 0, 31, 1, 0, 0, 35, 1, 0, 0, 65, 0, 5, 0, 16, 1, 0, 0, 37, 1, 0, 0, 30, 1, 0, 0, 189, 0, 0, 0, 62, 0, 3, 0, 37, 1, 0, 0, 36, 1, 0, 0, 65, 0, 5, 0, 14, 0, 0, 0, 38, 1, 0, 0, 147, 0, 0, 0, 28, 0, 0, 0, 61, 0, 4, 0, 6, 0, 0, 0, 39, 1, 0, 0, 38, 1, 0, 0, 65, 0, 6, 0, 23, 1, 0, 0, 40, 1, 0, 0, 30, 1, 0, 0, 189, 0, 0, 0, 28, 0, 0, 0, 62, 0, 3, 0, 40, 1, 0, 0, 39, 1, 0, 0, 253, 0, 1, 0, 56, 0, 1, 0};

struct ShaderSourceCode {
    ShaderSourceCode(const void* v, size_t vs, const void* f, size_t fs) :
        vs_source(v),
        vs_size(vs),
        fs_source(f),
        fs_size(fs) {}

    const void* vs_source;
    size_t vs_size;

    const void* fs_source;
    size_t fs_size;
};

static inline ShaderSourceCode GetFontShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_FONT_VERT, sizeof(VULKAN_FONT_VERT), VULKAN_FONT_FRAG, sizeof(VULKAN_FONT_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_FONT, sizeof(D3D11_FONT), D3D11_FONT, sizeof(D3D11_FONT));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_FONT, sizeof(METAL_FONT), METAL_FONT, sizeof(METAL_FONT));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_FONT_VERT, sizeof(GL_FONT_VERT), GL_FONT_FRAG, sizeof(GL_FONT_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetLineShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_LINE_VERT, sizeof(VULKAN_LINE_VERT), VULKAN_LINE_FRAG, sizeof(VULKAN_LINE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_LINE, sizeof(D3D11_LINE), D3D11_LINE, sizeof(D3D11_LINE));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_LINE, sizeof(METAL_LINE), METAL_LINE, sizeof(METAL_LINE));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_LINE_VERT, sizeof(GL_LINE_VERT), GL_LINE_FRAG, sizeof(GL_LINE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetNinepatchShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_NINEPATCH_VERT, sizeof(VULKAN_NINEPATCH_VERT), VULKAN_NINEPATCH_FRAG, sizeof(VULKAN_NINEPATCH_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_NINEPATCH, sizeof(D3D11_NINEPATCH), D3D11_NINEPATCH, sizeof(D3D11_NINEPATCH));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_NINEPATCH, sizeof(METAL_NINEPATCH), METAL_NINEPATCH, sizeof(METAL_NINEPATCH));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_NINEPATCH_VERT, sizeof(GL_NINEPATCH_VERT), GL_NINEPATCH_FRAG, sizeof(GL_NINEPATCH_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetShapeShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_SHAPE_VERT, sizeof(VULKAN_SHAPE_VERT), VULKAN_SHAPE_FRAG, sizeof(VULKAN_SHAPE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_SHAPE, sizeof(D3D11_SHAPE), D3D11_SHAPE, sizeof(D3D11_SHAPE));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_SHAPE, sizeof(METAL_SHAPE), METAL_SHAPE, sizeof(METAL_SHAPE));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_SHAPE_VERT, sizeof(GL_SHAPE_VERT), GL_SHAPE_FRAG, sizeof(GL_SHAPE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
static inline ShaderSourceCode GetSpriteShaderSourceCode(const sge::RenderBackend backend) {
    switch (backend) {
        case sge::RenderBackend::Vulkan: return ShaderSourceCode(VULKAN_SPRITE_VERT, sizeof(VULKAN_SPRITE_VERT), VULKAN_SPRITE_FRAG, sizeof(VULKAN_SPRITE_FRAG));
        case sge::RenderBackend::D3D11:
        case sge::RenderBackend::D3D12: return ShaderSourceCode(D3D11_SPRITE, sizeof(D3D11_SPRITE), D3D11_SPRITE, sizeof(D3D11_SPRITE));
        case sge::RenderBackend::Metal: return ShaderSourceCode(METAL_SPRITE, sizeof(METAL_SPRITE), METAL_SPRITE, sizeof(METAL_SPRITE));
        case sge::RenderBackend::OpenGL: return ShaderSourceCode(GL_SPRITE_VERT, sizeof(GL_SPRITE_VERT), GL_SPRITE_FRAG, sizeof(GL_SPRITE_FRAG));
        default: SGE_UNREACHABLE();
    }
}
#endif