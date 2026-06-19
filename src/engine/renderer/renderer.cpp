#include <SGE/assert.hpp>
#include <SGE/log.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/utils/alloc.hpp>

#include <LLGL/CommandBufferFlags.h>
#include <LLGL/Format.h>
#include <LLGL/PipelineCache.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Types.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/VertexAttribute.h>

sge::Renderer::Renderer(const std::shared_ptr<RenderContext>& context) : m_context(context) {
    SGE_ASSERT(context->GetLLGLContext() != nullptr);

    const RenderBackend backend = context->Backend();

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = m_context->GetCommandBuffer();
    m_command_queue = m_context->GetCommandQueue();

    m_uniform_buffer = m_context->CreateConstantBuffer(sizeof(GlobalUniforms), "Uniforms Buffer");
}

void sge::Renderer::BeginPass(LLGL::RenderTarget& target, const Camera& camera) {
    ZoneScoped;

    m_context->PushRenderTarget(&target);

    m_viewport = target.GetResolution();

    auto global_uniforms = GlobalUniforms {
        .screen_projection_matrix = camera.get_screen_projection_matrix(),
        .view_projection_matrix = camera.get_view_projection_matrix(),
        .inv_view_proj_matrix = camera.get_inv_view_projection_matrix(),
        .nonscale_view_projection_matrix = camera.get_nonscale_view_projection_matrix(),
        .nonscale_projection_matrix = camera.get_nonscale_projection_matrix(),
        .camera_position = camera.position(),
        .window_size = camera.viewport()
    };

    m_command_buffer->UpdateBuffer(*m_uniform_buffer, 0, &global_uniforms, sizeof(global_uniforms));

    m_command_buffer->BeginRenderPass(target);
    m_command_buffer->SetViewport(m_viewport);
}
