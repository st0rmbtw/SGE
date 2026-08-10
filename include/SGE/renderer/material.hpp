#ifndef SGE_RENDERER_MATERIAL_HPP_
#define SGE_RENDERER_MATERIAL_HPP_

#include <variant>

#include <LLGL/ResourceFlags.h>
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
    uint32_t index = 0;
    long bindFlags = 0;
    long stage = -1;
    uint32_t arraySize = 0;
    std::variant<
        sge::Ref<LLGL::Texture>,
        sge::Ref<LLGL::Buffer>,
        sge::Ref<LLGL::Sampler>
    > resource;
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

class Material {
public:
    Material() = default;

    Material&& SetVertexShader(sge::Ref<LLGL::Shader> vertexShader) && {
        m_vertex_shader = std::move(vertexShader);
        return std::move(*this);
    }

    Material&& SetFragmentShader(sge::Ref<LLGL::Shader> fragmentShader) && {
        m_fragment_shader = std::move(fragmentShader);
        return std::move(*this);
    }

    Material&& SetCullMode(sge::CullMode cullMode) && {
        m_cull_mode = cullMode;
        return std::move(*this);
    }

    Material&& SetBlendMode(sge::BlendMode blendMode) && {
        m_blend_mode = blendMode;
        return std::move(*this);
    }

    Material&& BindTextureStorage(uint32_t slot, sge::Ref<LLGL::Texture> texture, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
            .resource = texture
        });
        return std::move(*this);
    }

    Material&& BindTexture(uint32_t slot, sge::Ref<LLGL::Texture> texture, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
            .resource = texture
        });
        return std::move(*this);
    }

    Material&& BindStorageBuffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
            .resource = buffer
        });
        return std::move(*this);
    }

    Material&& BindBuffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
            .resource = buffer
        });
        return std::move(*this);
    }

    Material&& BindConstantBuffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::ConstantBuffer,
            .stage = stage,
            .resource = buffer
        });
        return std::move(*this);
    }

    Material&& BindSampler(uint32_t slot, sge::Ref<LLGL::Sampler> sampler, long stage = -1) && {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = 0,
            .stage = stage,
            .resource = sampler
        });
        return std::move(*this);
    }

    Material&& AddInstanceAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semanticName, uint32_t slot = 0) && {
        m_instance_attributes.emplace_back(format, std::move(name), std::move(semanticName), slot);
        return std::move(*this);
    }

    Material&& AddInstanceAttribute(sge::InstanceAttribute attribute) && {
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

} // namespace sge

#endif // SGE_RENDERER_MATERIAL_HPP_