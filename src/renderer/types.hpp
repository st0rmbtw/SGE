#ifndef RENDERER_TYPES
#define RENDERER_TYPES

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct BaseVertex {
    float x;
    float y;

    explicit BaseVertex(float x, float y) : x(x), y(y) {}
};
struct ShapeInstance {
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 color;
    glm::vec4 border_color;
    float border_thickness;
    float border_radius;
    int flags;
    uint32_t shape;
};

struct SpriteInstance {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::vec4 outline_color;
    float outline_thickness;
    int flags;
};

struct GlyphInstance {
    glm::vec3 color;
    glm::vec3 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 uv;
    int is_ui;
};

#endif