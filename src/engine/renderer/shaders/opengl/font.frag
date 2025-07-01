#version 330 core

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
}