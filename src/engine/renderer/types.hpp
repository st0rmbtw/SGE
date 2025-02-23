#ifndef _SGE_RENDERER_TYPES_HPP_
#define _SGE_RENDERER_TYPES_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../defines.hpp"

_SGE_BEGIN

namespace renderer {

struct Vertex {
    float x;
    float y;

    explicit Vertex(float x, float y) : x(x), y(y) {}
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

struct NinePatchInstance {
    glm::quat rotation;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec2 source_size;
    glm::vec2 output_size;
    glm::uvec4 margin;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    int flags;
};

struct GlyphInstance {
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 uv;
};

struct ShapeInstance {
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 color;
    glm::vec4 border_color;
    float border_thickness;
    float border_radius;
    uint32_t shape;
};

}

_SGE_END

#endif