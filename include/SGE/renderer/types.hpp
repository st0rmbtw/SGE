#ifndef _SGE_RENDERER_TYPES_HPP_
#define _SGE_RENDERER_TYPES_HPP_

#include <LLGL/PipelineStateFlags.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <LLGL/LLGL.h>
#include <SGE/assert.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/resource.hpp>

namespace sge {


template <typename T>
class Handle {
public:
    Handle() = default;
    explicit Handle(uint32_t id) : m_id(id) {
        SGE_ASSERT(id != static_cast<uint32_t>(-1));
    }

    [[nodiscard]]
    inline uint32_t ID() const noexcept {
        return m_id;
    }

    [[nodiscard]]
    inline bool IsValid() const noexcept {
        return m_id != static_cast<uint32_t>(-1);
    }

private:
    uint32_t m_id = -1;
};

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
    std::string debugName;
    LLGL::BlendDescriptor blend;
    LLGL::DepthDescriptor depth;
    Ref<LLGL::PipelineLayout> layout;
    Ref<LLGL::Shader> vertexShader;
    Ref<LLGL::Shader> geometryShader;
    Ref<LLGL::Shader> pixelShader;
    Handle<LLGL::RenderPass> renderPass;
    LLGL::PrimitiveTopology primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
    LLGL::Format indexFormat = LLGL::Format::Undefined;
    LLGL::CullMode cullMode = LLGL::CullMode::Disabled;
    bool frontCCW = true;
    bool scissorTestEnabled = false;
};

struct ComputePipelineConfig {
    const char* debugName = nullptr;
    LLGL::PipelineLayout* pipelineLayout = nullptr;
    LLGL::Shader* computeShader = nullptr;
};

struct AttachmentConfig {
    AttachmentConfig() = default;
    explicit AttachmentConfig(const sge::Ref<LLGL::Texture>& t) :
        format(t->GetFormat()),
        texture(t)
    {}

    LLGL::Format format = LLGL::Format::Undefined;
    sge::Ref<LLGL::Texture> texture = nullptr;
    uint32_t mipLevel = 0;
    uint32_t arrayLayer = 0;
};

struct RenderTargetConfig {
    std::string debugName;
    Handle<LLGL::RenderPass> renderPass;
    LLGL::Extent2D resolution;
    LLGL::Format format;

    sge::AttachmentConfig colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    sge::AttachmentConfig depthStencilAttachment;
};

struct RenderPassConfig {
    std::string debugName;
    LLGL::AttachmentFormatDescriptor colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    LLGL::AttachmentFormatDescriptor depthAttachment;
    LLGL::AttachmentFormatDescriptor stencilAttachment;
};

struct SpriteBatchPipeline {
    sge::Handle<LLGL::PipelineState> additive;
    sge::Handle<LLGL::PipelineState> alpha_blend;
    sge::Handle<LLGL::PipelineState> opaque;
    sge::Handle<LLGL::PipelineState> premultiplied_alpha;

    sge::Handle<LLGL::PipelineState> depth_additive;
    sge::Handle<LLGL::PipelineState> depth_alpha_blend;
    sge::Handle<LLGL::PipelineState> depth_opaque;
    sge::Handle<LLGL::PipelineState> depth_premultiplied_alpha;
};

}

#endif