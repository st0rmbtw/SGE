#version 330 core

out vec4 frag_color;

flat in vec4 v_color;

void main() {
    if (v_color.a <= 0.05) discard;

    frag_color = v_color;
}