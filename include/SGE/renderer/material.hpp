#ifndef SGE_RENDERER_MATERIAL_HPP_
#define SGE_RENDERER_MATERIAL_HPP_

#include <variant>

#include <LLGL/ResourceFlags.h>
#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/vertex_attribute.hpp>
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

class Material {
public:
    Material() = default;

    Material& SetVertexShader(sge::Ref<LLGL::Shader> vertex_shader) {
        m_vertex_shader = std::move(vertex_shader);
        return *this;
    }

    Material& SetFragmentShader(sge::Ref<LLGL::Shader> fragment_shader) {
        m_fragment_shader = std::move(fragment_shader);
        return *this;
    }

    Material& SetVertexEntryPoint(LLGL::StringLiteral entry_point) {
        m_vertex_entry = std::move(entry_point);
        return *this;
    }

    Material& SetFragmentEntryPoint(LLGL::StringLiteral entry_point) {
        m_fragment_entry = std::move(entry_point);
        return *this;
    }

    Material& SetCullMode(CullMode cull_mode) {
        m_cull_mode = cull_mode;
        return *this;
    }

    Material& SetBlendMode(BlendMode blend_mode) {
        m_blend_mode = blend_mode;
        return *this;
    }

    Material& TextureStorage(uint32_t slot, sge::Ref<LLGL::Texture> texture, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
            .resource = texture
        });
        return *this;
    }

    Material& Texture(uint32_t slot, sge::Ref<LLGL::Texture> texture, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
            .resource = texture
        });
        return *this;
    }

    Material& StorageBuffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Storage,
            .stage = stage,
            .resource = buffer
        });
        return *this;
    }

    Material& Buffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::Sampled,
            .stage = stage,
            .resource = buffer
        });
        return *this;
    }

    Material& ConstantBuffer(uint32_t slot, sge::Ref<LLGL::Buffer> buffer, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = LLGL::BindFlags::ConstantBuffer,
            .stage = stage,
            .resource = buffer
        });
        return *this;
    }

    Material& Sampler(uint32_t slot, sge::Ref<LLGL::Sampler> sampler, long stage = -1) {
        m_bindings.push_back(BindingSlot {
            .index = slot,
            .bindFlags = 0,
            .stage = stage,
            .resource = sampler
        });
        return *this;
    }

    Material& AddInstanceAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) {
        m_instance_attributes.emplace_back(format, std::move(name), std::move(semantic_name), slot);
        return *this;
    }

    Material& AddInstanceAttribute(sge::VertexAttribute attribute) {
        m_instance_attributes.push_back(std::move(attribute));
        return *this;
    }

    [[nodiscard]]
    const std::vector<VertexAttribute>& GetInstanceAttributes() const {
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
    const LLGL::StringLiteral& GetVertexEntry() const {
        return m_vertex_entry;
    }

    [[nodiscard]]
    const LLGL::StringLiteral& GetFragmentEntry() const {
        return m_vertex_entry;
    }

    [[nodiscard]]
    CullMode GetCullMode() const {
        return m_cull_mode;
    }

    [[nodiscard]]
    BlendMode GetBlendMode() const {
        return m_blend_mode;
    }

private:
    std::vector<VertexAttribute> m_instance_attributes;
    std::vector<BindingSlot> m_bindings;

    sge::Ref<LLGL::Shader> m_vertex_shader = nullptr;
    sge::Ref<LLGL::Shader> m_fragment_shader = nullptr;

    LLGL::StringLiteral m_vertex_entry = "VS";
    LLGL::StringLiteral m_fragment_entry = "PS";

    CullMode m_cull_mode = CullMode::Back;
    BlendMode m_blend_mode = BlendMode::AlphaBlend;
};

} // namespace sge

#endif // SGE_RENDERER_MATERIAL_HPP_