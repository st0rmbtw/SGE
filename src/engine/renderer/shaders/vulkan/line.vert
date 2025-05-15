#version 450 core

layout(location = 0) in vec2 a_position;

layout(location = 1) in vec2 i_start;
layout(location = 2) in vec2 i_end;
layout(location = 3) in vec4 i_color;
layout(location = 4) in vec4 i_border_radius;
layout(location = 5) in float i_thickness;
layout(location = 6) in uint i_flags;

layout(binding = 2) uniform GlobalUniformBuffer {
    mat4 screen_projection;
    mat4 view_projection;
    mat4 nonscale_view_projection;
    mat4 nonscale_projection;
    mat4 transform_matrix;
    mat4 inv_view_proj;
    vec2 camera_position;
    vec2 window_size;
} global_ubo;

layout(location = 0) flat out vec4 v_color;
layout(location = 1) flat out vec4 v_border_radius;
layout(location = 2) out vec2 v_point;
layout(location = 3) out vec2 v_uv;
layout(location = 4) flat out vec2 v_size;

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
    gl_Position = mvp * vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    gl_Position.z = 1.0;
}