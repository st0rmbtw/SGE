#ifndef _SGE_RENDERER_TYPES_HPP_
#define _SGE_RENDERER_TYPES_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <LLGL/LLGL.h>
#include <SGE/renderer/macros.hpp>

namespace sge {

struct Vertex {
    float x;
    float y;

    explicit Vertex(float x, float y) noexcept : x(x), y(y) {}
};

struct LineInstance {
    glm::vec2 start;
    glm::vec2 end;
    glm::vec4 color;
    glm::vec4 border_radius;
    float thickness;
    uint8_t flags;
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
    uint8_t flags;
};

struct NinePatchInstance {
    glm::vec2 position;
    glm::quat rotation;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec2 source_size;
    glm::vec2 output_size;
    glm::uvec4 margin;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    uint8_t flags;
};

struct GlyphInstance {
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 uv;
    uint8_t flags;
};

struct ShapeInstance {
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec4 color;
    glm::vec4 border_color;
    glm::vec4 border_radius;
    float border_thickness;
    uint8_t shape;
    uint8_t flags;
};

struct SpriteBatchPipeline {
    LLGL::PipelineState* additive = nullptr;
    LLGL::PipelineState* alpha_blend = nullptr;
    LLGL::PipelineState* opaque = nullptr;
    LLGL::PipelineState* premultiplied_alpha = nullptr;

    LLGL::PipelineState* depth_additive = nullptr;
    LLGL::PipelineState* depth_alpha_blend = nullptr;
    LLGL::PipelineState* depth_opaque = nullptr;
    LLGL::PipelineState* depth_premultiplied_alpha = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        SGE_RESOURCE_RELEASE(additive);
        SGE_RESOURCE_RELEASE(alpha_blend);
        SGE_RESOURCE_RELEASE(opaque);
        SGE_RESOURCE_RELEASE(premultiplied_alpha);
        SGE_RESOURCE_RELEASE(depth_additive);
        SGE_RESOURCE_RELEASE(depth_alpha_blend);
        SGE_RESOURCE_RELEASE(depth_opaque);
        SGE_RESOURCE_RELEASE(depth_premultiplied_alpha);
    }
};

}

#endif