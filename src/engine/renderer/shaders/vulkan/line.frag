#version 450 core

layout(location = 0) out vec4 frag_color;

layout(location = 0) flat in vec4 v_color;

void main() {
    if (v_color.a <= 0.05) discard;

    frag_color = v_color;
}