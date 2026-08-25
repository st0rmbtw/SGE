#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Sampler.h>

#include <SGE/assert.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/utils/hash.hpp>

#include "utils.hpp"

sge::Material::Material(sge::RenderContext& context, const sge::MaterialDesc& desc) :
    m_vertex_shader{ desc.GetVertexShader() },
    m_fragment_shader{ desc.GetFragmentShader() },
    m_blend_mode{ desc.GetBlendMode() },
    m_cull_mode{ desc.GetCullMode() },
    m_scissor_test_enabled{ desc.IsScissorTestEnabled() },
    m_depth_test_enabled{ desc.IsDepthTestEnabled() }
{
    uint64_t stateHash = sge::HASH_INIT;
    sge::hash_combine(stateHash, desc.GetCullMode());
    sge::hash_combine(stateHash, desc.GetBlendMode());

    LLGL::VertexFormat instanceFormat = ConvertInstanceAttributesToLLGL(context.Backend(), desc.GetInstanceAttributes());
    HashVertexAttributes(stateHash, instanceFormat.attributes);

    std::vector<LLGL::ResourceViewDescriptor> resourceViews;
    LLGL::PipelineLayoutDescriptor layoutDesc;
    for (const auto& binding : desc.GetBindings()) {
        auto resourceType = LLGL::ResourceType::Undefined;
        auto resourceView = LLGL::ResourceViewDescriptor();

        if (const auto* texture = std::get_if<sge::Ref<LLGL::Texture>>(&binding.resource)) {
            resourceType = LLGL::ResourceType::Texture;
            resourceView = texture->Get();
        } else if (const auto* buffer = std::get_if<sge::Ref<LLGL::Buffer>>(&binding.resource)) {
            resourceType = LLGL::ResourceType::Buffer;
            resourceView = buffer->Get();
        } else if (const auto* sampler = std::get_if<sge::Ref<LLGL::Sampler>>(&binding.resource)) {
            resourceType = LLGL::ResourceType::Sampler;
            resourceView = sampler->Get();
        } else {
            SGE_UNREACHABLE();
        }

        sge::hash_fnv1a(stateHash, binding.name.data(), binding.name.size());
        sge::hash_combine(stateHash, resourceType);
        sge::hash_combine(stateHash, binding.index);
        sge::hash_combine(stateHash, binding.bindFlags);
        sge::hash_combine(stateHash, binding.stage);
        sge::hash_combine(stateHash, binding.arraySize);

        layoutDesc.heapBindings.emplace_back(binding.name, resourceType, binding.bindFlags, binding.stage, LLGL::BindingSlot(binding.index), binding.arraySize);
        resourceViews.emplace_back(resourceView);
    }

    for (const auto& binding : desc.GetDynamicBindings()) {
        sge::hash_fnv1a(stateHash, binding.name.data(), binding.name.size());
        sge::hash_combine(stateHash, binding.type);
        sge::hash_combine(stateHash, binding.index);
        sge::hash_combine(stateHash, binding.bindFlags);
        sge::hash_combine(stateHash, binding.stage);
        sge::hash_combine(stateHash, binding.arraySize);
        layoutDesc.bindings.emplace_back(binding.name, binding.type, binding.bindFlags, binding.stage, LLGL::BindingSlot(binding.index), binding.arraySize);
    }

    sge::Ref<LLGL::PipelineLayout> pipelineLayout = context.CreatePipelineLayout(layoutDesc);
    sge::Ref<LLGL::ResourceHeap> resourceHeap = context.CreateResourceHeap(pipelineLayout, resourceViews);

    m_instance_attribs = std::move(instanceFormat.attributes);
    m_pipeline_layout = std::move(pipelineLayout);
    m_resource_heap = std::move(resourceHeap);
    m_state_hash = stateHash;
}