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
public:
    MaterialDesc() = default;

    MaterialDesc&& SetVertexShader(sge::Ref<LLGL::Shader> vertexShader) && {
        m_vertex_shader = std::move(vertexShader);
        return std::move(*this);
    }

    MaterialDesc&& SetFragmentShader(sge::Ref<LLGL::Shader> fragmentShader) && {
        m_fragment_shader = std::move(fragmentShader);
        return std::move(*this);
    }

    MaterialDesc&& SetCullMode(sge::CullMode cullMode) && {
        m_cull_mode = cullMode;
        return std::move(*this);
    }

    MaterialDesc&& SetBlendMode(sge::BlendMode blendMode) && {
        m_blend_mode = blendMode;
        return std::move(*this);
    }

    MaterialDesc&& BindTextureStorage(uint32_t slot, sge::Ref<LLGL::Texture> texture, long stage) && {
        m_bindings.push_back(BindingSlot {
            .resource = texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& BindTexture(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Texture> texture, long stage) && {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = texture,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& BindStorageBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) && {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& BindBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) && {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& BindConstantBuffer(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Buffer> buffer, long stage) && {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = buffer,
            .index = slot,
            .bindFlags = LLGL::BindFlags::ConstantBuffer,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& BindSampler(uint32_t slot, LLGL::StringLiteral name, sge::Ref<LLGL::Sampler> sampler, long stage) && {
        m_bindings.push_back(BindingSlot {
            .name = std::move(name),
            .resource = sampler,
            .index = slot,
            .bindFlags = 0,
            .stage = stage,
        });
        return std::move(*this);
    }

    MaterialDesc&& AddInstanceAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semanticName, uint32_t slot = 0) && {
        m_instance_attributes.emplace_back(format, std::move(name), std::move(semanticName), slot);
        return std::move(*this);
    }

    MaterialDesc&& AddInstanceAttribute(sge::InstanceAttribute attribute) && {
        m_instance_attributes.push_back(std::move(attribute));
        return std::move(*this);
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

private:
    std::vector<InstanceAttribute> m_instance_attributes;
    std::vector<BindingSlot> m_bindings;

    sge::Ref<LLGL::Shader> m_vertex_shader = nullptr;
    sge::Ref<LLGL::Shader> m_fragment_shader = nullptr;

    CullMode m_cull_mode = CullMode::None;
    BlendMode m_blend_mode = BlendMode::AlphaBlend;
};

class Material {
public:
    explicit Material(sge::RenderContext& context, const MaterialDesc& desc);

    [[nodiscard]]
    const std::vector<LLGL::VertexAttribute>& GetInstanceAttribs() const {
        return m_instance_attribs;
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

private:
    std::vector<LLGL::VertexAttribute> m_instance_attribs;
    sge::Ref<LLGL::Shader> m_vertex_shader;
    sge::Ref<LLGL::Shader> m_fragment_shader;
    sge::Ref<LLGL::PipelineLayout> m_pipeline_layout;
    sge::Ref<LLGL::ResourceHeap> m_resource_heap;
    uint64_t m_state_hash = 0;
    sge::BlendMode m_blend_mode;
    sge::CullMode m_cull_mode;
};

} // namespace sge

#endif // SGE_RENDERER_MATERIAL_HPP_