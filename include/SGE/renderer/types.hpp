#ifndef _SGE_RENDERER_TYPES_HPP_
#define _SGE_RENDERER_TYPES_HPP_

#include <LLGL/FragmentAttribute.h>
#include <LLGL/LLGL.h>
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/Utils/VertexFormat.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SGE/assert.hpp>
#include <SGE/renderer/handle.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/resource.hpp>
#include <SGE/types/sampler.hpp>
#include <SGE/types/shader_def.hpp>

namespace sge {

struct GpuMesh {
    LLGL::VertexFormat vertexFormat;
    sge::Unique<LLGL::Buffer> vertexBuffer;
    sge::Unique<LLGL::Buffer> indexBuffer;
    uint64_t layoutHash = 0;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    sge::IndexFormat indexFormat;
    sge::PrimitiveTopology topology;
    sge::FrontFace frontFace;
};

struct Vertex {
    float x;
    float y;

    explicit Vertex(float x, float y) noexcept : x(x), y(y) {}
};

#pragma pack(push, 1)

struct LineInstance {
    glm::vec2 start;
    glm::vec2 end;
    glm::vec4 color;
    glm::vec4 border_radius;
    float thickness;
    uint8_t flags;
};

struct SpriteInstance {
    glm::quat rotation;
    glm::vec4 uv_offset_scale;
    glm::vec4 color;
    glm::vec4 outline_color;
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    float outline_thickness;
    uint8_t flags;
};

struct NinePatchInstance {
    glm::quat rotation;
    glm::vec4 color;
    glm::vec4 uv_offset_scale;
    glm::uvec4 margin;
    glm::vec2 position;
    glm::vec2 offset;
    glm::vec2 source_size;
    glm::vec2 output_size;
    uint8_t flags;
};

struct GlyphInstanceVector {
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 em_size;
    uint32_t partition_offset;
    uint32_t partition_count;
    uint8_t flags;
};

struct GlyphInstanceSDF {
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 tex_size;
    glm::vec2 uv;
    uint8_t flags;
};

struct ShapeInstance {
    glm::vec4 color;
    glm::vec4 border_color;
    glm::vec4 border_radius;
    glm::vec3 position;
    glm::vec2 size;
    glm::vec2 offset;
    float border_thickness;
    uint8_t shape;
    uint8_t flags;
};

#pragma pack(pop)

struct PathData {
    glm::mat4 transformMatrix;
    glm::vec4 color;
};

namespace internal {

template <typename Tag>
class ResourceId {
    static inline constexpr uint32_t INVALID = 0;
public:
    ResourceId() = default;
    explicit ResourceId(uint32_t value) : m_value(value) {}

    [[nodiscard]]
    uint32_t Value() const noexcept {
        return m_value;
    }

    [[nodiscard]]
    bool IsValid() const noexcept {
        return m_value != INVALID;
    }

    friend bool operator==(const ResourceId& a, const ResourceId& b) noexcept { return a.m_value == b.m_value; }
    friend bool operator!=(const ResourceId& a, const ResourceId& b) noexcept { return a.m_value != b.m_value; }
private:
    uint32_t m_value = INVALID;
};

} // namespace internal

using PipelineId = internal::ResourceId<LLGL::PipelineState>;
using RenderPassId = internal::ResourceId<LLGL::RenderPass>;
using RenderTargetId = internal::ResourceId<LLGL::RenderTarget>;

struct GraphicsPipelineConfig {
    LLGL::BlendDescriptor blend;
    LLGL::StencilDescriptor stencil;
    std::string debugName;
    LLGL::DepthDescriptor depth;
    Ref<LLGL::PipelineLayout> layout;
    Ref<LLGL::Shader> vertexShader;
    Ref<LLGL::Shader> geometryShader;
    Ref<LLGL::Shader> pixelShader;
    std::vector<LLGL::VertexAttribute> inputVertexAttribs;
    std::vector<LLGL::VertexAttribute> outputVertexAttribs;
    RenderPassId renderPass;
    sge::PrimitiveTopology primitiveTopology = sge::PrimitiveTopology::TriangleList;
    sge::IndexFormat indexFormat = sge::IndexFormat::None;
    sge::CullMode cullMode = sge::CullMode::None;
    sge::FrontFace frontFace = sge::FrontFace::CCW;
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
    long bindFlags = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment);
    uint32_t mipLevel = 0;
    uint32_t arrayLayer = 0;
};

struct RenderTargetConfig {
    std::string debugName;
    RenderPassId renderPass;
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

struct TextureConfig {
    const char* debugName = nullptr;
    LLGL::TextureType textureType = LLGL::TextureType::Texture2D;
    LLGL::Extent3D extent = LLGL::Extent3D(1, 1, 1);
    sge::Ref<sge::Sampler> sampler;
    LLGL::Format format = LLGL::Format::RGBA8UNorm;
    uint32_t arrayLayers = 1;
    bool generateMipMaps = false;
};

struct FramebufferConfig {
    sge::AttachmentConfig colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
    sge::AttachmentConfig depthStencilAttachment;

    const char* debugName = nullptr;
    sge::Ref<LLGL::RenderPass> renderPass;
    LLGL::Extent2D resolution;
    uint8_t samples = 1;
};

struct ShaderConfig {
    LLGL::VertexShaderAttributes vertex;
    LLGL::FragmentShaderAttributes fragment;
};

struct BloomSettings {
    float threshold = 1.0f;
    float knee = 0.5f;
    float intensity = 1.0f;
    float scatter = 1.0f;
    uint8_t maxIterations = 6;
};

inline constexpr bool operator==(const BloomSettings& a, const BloomSettings& b) noexcept {
    return a.threshold == b.threshold &&
           a.knee == b.knee &&
           a.intensity == b.intensity &&
           a.scatter == b.scatter &&
           a.maxIterations == b.maxIterations;
}

struct SpriteBatchPipeline {
    sge::PipelineId additive;
    sge::PipelineId alpha_blend;
    sge::PipelineId opaque;
    sge::PipelineId premultiplied_alpha;

    sge::PipelineId depth_additive;
    sge::PipelineId depth_alpha_blend;
    sge::PipelineId depth_opaque;
    sge::PipelineId depth_premultiplied_alpha;
};

struct TextureWithSampler {
    LLGL::Texture* texture = nullptr;
    LLGL::Sampler* sampler = nullptr;
};

} // namespace sge

template<typename Tag>
struct std::hash<sge::internal::ResourceId<Tag>> {
    size_t operator()(const sge::internal::ResourceId<Tag>& r) const noexcept {
        return std::hash<uint32_t>{}(r.Value());
    }
};

#endif