#ifndef _SGE_RENDERER_TYPES_HPP_
#define _SGE_RENDERER_TYPES_HPP_

#include "LLGL/PipelineStateFlags.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <LLGL/LLGL.h>
#include <SGE/renderer/macros.hpp>
#include <SGE/utils/llgl.hpp>

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

struct GraphicsPipelineConfig {
    LLGL::BlendDescriptor blend;
    LLGL::DepthDescriptor depth;
    const char* debugName = nullptr;
    LLGL::PipelineLayout* layout = nullptr;
    LLGL::Shader* vertexShader = nullptr;
    LLGL::Shader* geometryShader = nullptr;
    LLGL::Shader* pixelShader = nullptr;
    LLGL::PrimitiveTopology primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
    LLGL::Format indexFormat = LLGL::Format::Undefined;
    bool frontCCW = false;
    bool scissorTestEnabled = false;
};

struct SpriteBatchPipeline {
    std::optional<uint32_t> additive = -1;
    std::optional<uint32_t> alpha_blend = -1;
    std::optional<uint32_t> opaque = -1;
    std::optional<uint32_t> premultiplied_alpha = -1;

    std::optional<uint32_t> depth_additive = -1;
    std::optional<uint32_t> depth_alpha_blend = -1;
    std::optional<uint32_t> depth_opaque = -1;
    std::optional<uint32_t> depth_premultiplied_alpha = -1;
};

}

#endif