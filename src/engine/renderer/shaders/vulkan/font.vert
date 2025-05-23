#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 i_color;
layout(location = 2) in vec3 i_position;
layout(location = 3) in vec2 i_size;
layout(location = 4) in vec2 i_tex_size;
layout(location = 5) in vec2 i_uv;
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

layout(location = 0) out vec2 v_uv;
layout(location = 1) flat out vec3 v_color;

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
}