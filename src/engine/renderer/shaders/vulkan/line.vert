#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in uint a_flags;

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

const uint FLAG_UI = 1u << 0u;

void main() {
    bool is_ui = (a_flags & FLAG_UI) == FLAG_UI;

    mat4 mvp = is_ui ? global_ubo.screen_projection : global_ubo.view_projection;

    v_color = a_color;
    gl_Position = mvp * vec4(a_position, 0.0, 1.0);
    gl_Position.z = 1.0;
}