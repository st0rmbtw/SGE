#ifndef SGE_RENDERER_MATERIAL_HPP_
#define SGE_RENDERER_MATERIAL_HPP_

#include <variant>

#include <LLGL/PipelineLayout.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/ResourceHeap.h>

#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/vertex_format.hpp>
#include <SGE/types/binding_layout.hpp>

#include <LLGL/Buffer.h>
#include <LLGL/Sampler.h>
#include <LLGL/Shader.h>
#include <LLGL/Texture.h>


namespace sge {

enum class CullMode : uint8_t {
    None,
    Back,
    Front
};

enum class BlendMode : uint8_t {
    Opaque,
    AlphaBlend,
    Additive,
    PremultipliedAlpha
};

struct BindingSlot {
    LLGL::StringLiteral name;
    std::variant<
        sge::Ref<LLGL::Texture>,
        sge::Ref<LLGL::Buffer>,
        sge::Ref<LLGL::Sampler>
    > resource;
    uint32_t index = 0;
    long bindFlags = 0;
    long stage = 0;
    uint32_t arraySize = 0;
};

struct DynamicBindingSlot {
    LLGL::StringLiteral name;
    LLGL::ResourceType type;
    uint32_t index = 0;
    long bindFlags = 0;
    long stage = 0;
    uint32_t arraySize = 0;
};

struct InstanceAttribute {
    explicit InstanceAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot) :
        name(std::move(name)),
        semanticName(std::move(semantic_name)),
        slot(slot),
        format(format)
    {
    }

    LLGL::StringLiteral name;
    LLGL::StringLiteral semanticName;

    uint32_t slot = 0;
    sge::VertexFormat format;
};

class MaterialDesc {
    friend class Material;
public:
    MaterialDesc() = default;

    MaterialDesc& SetVertexShader(sge::Ref<LLGL::Shader> vertexShader) {
        m_vertex_shader = std::move(vertexShader);
        return *this;
    }

    MaterialDesc& SetFragmentShader(sge::Ref<LLGL::Shader> fragmentShader) {
        m_fragment_shader = std::move(fragmentShader);
        return *this;
    }

    MaterialDesc& SetCullMode(sge::CullMode cullMode) {
        m_cull_mode = cullMode;
        return *this;
    }

    MaterialDesc& SetBlendMode(sge::BlendMode blendMode) {
        m_blend_mode = blendMode;
        return *this;
    }

    MaterialDesc& BindTextureStorage(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Texture> texture, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& BindTexture(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Texture> texture, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& BindStorageBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& BindBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& BindConstantBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::ConstantBuffer,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& BindSampler(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Sampler> sampler, long stage) {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = sampler,
            .index = slot,
            .bindFlags = 0,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicTextureStorage(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicTexture(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicStorageBuffer(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicBuffer(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicConstantBuffer(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::ConstantBuffer,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddDynamicSampler(uint32_t slot, LLGL::StringLiteral name, long stage) {
        m_dynamic_bindings.push_back(DynamicBindingSlot {
            .name = std::move(name),
            .type = LLGL::ResourceType::Sampler,
            .index = slot,
            .bindFlags = 0,
            .stage = stage,
        });
        return *this;
    }

    MaterialDesc& AddInstanceAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semanticName) {
        m_instance_attributes.emplace_back(format, std::move(name), std::move(semanticName), 1);
        return *this;
    }

    MaterialDesc& AddInstanceAttribute(sge::InstanceAttribute attribute) {
        m_instance_attributes.push_back(std::move(attribute));
        return *this;
    }

    MaterialDesc& SetScissorTestEnabled(bool enabled) {
        m_scissor_test_enabled = enabled;
        return *this;
    }

    MaterialDesc& SetDepthTestEnabled(bool enabled) {
        m_depth_test_enabled = enabled;
        return *this;
    }

    [[nodiscard]]
    const std::vector<InstanceAttribute>& GetInstanceAttributes() const {
        return m_instance_attributes;
    }

    [[nodiscard]]
    const std::vector<BindingSlot>& GetBindings() const {
        return m_bindings;
    }

    [[nodiscard]]
    const std::vector<DynamicBindingSlot>& GetDynamicBindings() const {
        return m_dynamic_bindings;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::Shader>& GetVertexShader() const {
        return m_vertex_shader;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::Shader>& GetFragmentShader() const {
        return m_fragment_shader;
    }

    [[nodiscard]]
    sge::CullMode GetCullMode() const {
        return m_cull_mode;
    }

    [[nodiscard]]
    sge::BlendMode GetBlendMode() const {
        return m_blend_mode;
    }

    [[nodiscard]]
    bool IsScissorTestEnabled() const {
        return m_scissor_test_enabled;
    }

    [[nodiscard]]
    bool IsDepthTestEnabled() const {
        return m_depth_test_enabled;
    }

private:
    std::vector<InstanceAttribute> m_instance_attributes;
    std::vector<BindingSlot> m_bindings;
    std::vector<DynamicBindingSlot> m_dynamic_bindings;

    sge::Ref<LLGL::Shader> m_vertex_shader = nullptr;
    sge::Ref<LLGL::Shader> m_fragment_shader = nullptr;

    CullMode m_cull_mode = CullMode::None;
    BlendMode m_blend_mode = BlendMode::AlphaBlend;

    bool m_scissor_test_enabled = false;
    bool m_depth_test_enabled = false;
};

class Material : public RefCounted {
public:
    explicit Material(sge::RenderContext& context, MaterialDesc desc);

    [[nodiscard]]
    const std::vector<LLGL::VertexAttribute>& GetInstanceAttribs() const {
        return m_instance_attribs;
    }

    [[nodiscard]]
    uint32_t GetInstanceAttribsStride() const noexcept {
        return !m_instance_attribs.empty() ? m_instance_attribs[0].stride : 0;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::Shader>& GetVertexShader() const {
        return m_vertex_shader;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::Shader>& GetFragmentShader() const {
        return m_fragment_shader;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::PipelineLayout>& GetPipelineLayout() const {
        return m_pipeline_layout;
    }

    [[nodiscard]]
    const sge::Ref<LLGL::ResourceHeap>& GetResourceHeap() const {
        return m_resource_heap;
    }

    [[nodiscard]]
    uint64_t GetStateHash() const {
        return m_state_hash;
    }

    [[nodiscard]]
    sge::BlendMode GetBlendMode() const {
        return m_blend_mode;
    }

    [[nodiscard]]
    sge::CullMode GetCullMode() const {
        return m_cull_mode;
    }

    [[nodiscard]]
    bool IsScissorTestEnabled() const {
        return m_scissor_test_enabled;
    }

    [[nodiscard]]
    bool IsDepthTestEnabled() const {
        return m_depth_test_enabled;
    }

private:
    std::vector<LLGL::VertexAttribute> m_instance_attribs;
    sge::Ref<LLGL::Shader> m_vertex_shader;
    sge::Ref<LLGL::Shader> m_fragment_shader;
    sge::Ref<LLGL::PipelineLayout> m_pipeline_layout;
    sge::Ref<LLGL::ResourceHeap> m_resource_heap;
    uint64_t m_state_hash = 0;
    sge::BlendMode m_blend_mode;
    sge::CullMode m_cull_mode;
    bool m_scissor_test_enabled = false;
    bool m_depth_test_enabled = false;
};

} // namespace sge

#endif // SGE_RENDERER_MATERIAL_HPP_