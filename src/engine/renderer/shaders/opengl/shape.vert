#version 330 core

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
    v_point = (a_position - 0.5) * i_size;
    v_color = i_color;
    v_size = i_size;
    v_border_color = i_border_color;
    v_border_thickness = i_border_thickness;
    v_border_radius = i_border_radius;
    v_shape = i_shape;
    
    gl_Position = mvp * vec4(a_position, 0, 1);
    gl_Position.z = i_position.z;
}