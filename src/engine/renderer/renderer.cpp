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

#include "SGE/types/framebuffer.hpp"
#include "shaders.hpp"

namespace {

struct BloomUniforms {
    float threshold;
    float knee;
    float filterRadius;
    float intensity;
};

} // namespace

sge::Renderer::Renderer(const std::shared_ptr<RenderContext>& context) : m_context(context) {
    SGE_ASSERT(context->GetLLGLContext() != nullptr);

    const RenderBackend backend = context->Backend();

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = m_context->GetCommandBuffer();
    m_command_queue = m_context->GetCommandQueue();

    m_uniform_buffer = m_context->CreateConstantBuffer(sizeof(GlobalUniforms), "Uniforms Buffer");

    m_fullscreen_triangle_vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_position", "Position"),
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_uv", "UV")
    });

    const glm::vec2 vertices[] = {
        glm::vec2(-1.0f, 1.0f),  glm::vec2(0.0f, 0.0f),
        glm::vec2(3.0f,  1.0f),  glm::vec2(2.0f, 0.0f),
        glm::vec2(-1.0f, -3.0f), glm::vec2(0.0f, 2.0f),
    };
    m_fullscreen_triangle_vertex_buffer = context->CreateVertexBuffer(vertices, m_fullscreen_triangle_vertex_format);

    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = m_fullscreen_triangle_vertex_format.attributes;

    ShaderSourceCode shader = GetFullscreenTriangleShaderSourceCode(backend);
    m_fullscreen_triangle_vertex_shader = context->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
    m_blit_pixel_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::Texture(2, "SourceTexture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(3, "SourceSampler", LLGL::StageFlags::FragmentStage)
    });
    m_blit_pipeline_layout = m_context->CreatePipelineLayout(layoutDesc);

    LLGL::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachments[0].format = LLGL::Format::RGBA8UNorm;
    renderPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    m_blit_render_pass = m_context->CreateRenderPass(renderPassDesc);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.pipelineLayout = m_blit_pipeline_layout;
    pipelineDesc.vertexShader = m_fullscreen_triangle_vertex_shader;
    pipelineDesc.fragmentShader = m_blit_pixel_shader;
    pipelineDesc.renderPass = m_blit_render_pass;
    pipelineDesc.indexFormat = LLGL::Format::Undefined;
    pipelineDesc.depth.testEnabled = false;
    pipelineDesc.depth.writeEnabled = false;
    m_blit_pipeline = m_context->CreatePipelineState(pipelineDesc);

    m_bloom_cb = m_context->CreateConstantBuffer(sizeof(BloomUniforms), "Bloom Constant Buffer");
}

void sge::Renderer::BeginPass(LLGL::RenderTarget& target, const Camera& camera) {
    ZoneScoped;

    m_context->PushRenderTarget(&target);

    m_viewport = target.GetResolution();

    auto global_uniforms = GlobalUniforms {
        .screen_projection_matrix = camera.get_screen_projection_matrix(),
        .view_projection_matrix = camera.get_view_projection_matrix(),
        .inv_view_proj_matrix = camera.get_inv_view_projection_matrix(),
        .camera_position = camera.transform().translation,
        .window_size = camera.viewport()
    };

    m_command_buffer->UpdateBuffer(*m_uniform_buffer, 0, &global_uniforms, sizeof(global_uniforms));

    m_command_buffer->BeginRenderPass(target);
    m_command_buffer->SetViewport(m_viewport);
}

void sge::Renderer::InitTonemapPipelines() {
    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::Texture(4, "MainTexture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(5, "MainSampler", LLGL::StageFlags::FragmentStage),
    });

    LLGL::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachments[0].format = LLGL::Format::RG11B10Float;
    renderPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    m_tonemap_render_pass = m_context->CreateRenderPass(renderPassDesc);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineDesc.renderPass = m_tonemap_render_pass;
    pipelineDesc.vertexShader = FullscreenTriangleVertexShader();
    pipelineDesc.indexFormat = LLGL::Format::Undefined;
    pipelineDesc.depth.testEnabled = false;
    pipelineDesc.depth.writeEnabled = false;

    ShaderSourceCode shaderSource = GetTonemapAcesShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size);
    m_tonemap_aces_pipeline = m_context->CreatePipelineState(pipelineDesc);
}

void sge::Renderer::TonemapPass(sge::Framebuffer& framebuffer) {
    if (!m_tonemap_aces_pipeline)
        InitTonemapPipelines();

    LLGL::Extent2D resolution = framebuffer.GetResolution();

    auto tonemap_framebuffer = m_context->GetTemporaryFramebuffer(resolution, LLGL::Format::RG11B10Float);

    m_command_buffer->SetViewport(resolution);
    BeginPass(*tonemap_framebuffer.GetRenderTarget());
    {
        m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
        m_command_buffer->SetPipelineState(*BlitPipeline());
        m_command_buffer->SetResource(0, *framebuffer.GetTexture(0));
        m_command_buffer->SetResource(1, *m_context->GetLinearSampler());
        m_command_buffer->Draw(3, 0);
    }
    EndPass();

    m_command_buffer->SetViewport(resolution);
    BeginPass(*framebuffer.GetRenderTarget());
    {
        m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
        m_command_buffer->SetPipelineState(*m_tonemap_aces_pipeline);
        m_command_buffer->SetResource(0, *tonemap_framebuffer.GetTexture(0));
        m_command_buffer->SetResource(1, *m_context->GetLinearSampler());
        m_command_buffer->Draw(3, 0);
    }
    EndPass();
}

void sge::Renderer::InitBloomPipelines() {
    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(3, "BloomConstantBuffer", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Texture(4, "MainTexture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(5, "MainSampler", LLGL::StageFlags::FragmentStage),
    });

    LLGL::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachments[0].format = LLGL::Format::RG11B10Float;
    renderPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    m_bloom_render_pass = m_context->CreateRenderPass(renderPassDesc);
    
    sge::ShaderConfig shaderConfig;
    shaderConfig.fragment.outputAttribs = {
        LLGL::FragmentAttribute{ "SV_Target", LLGL::Format::RG11B10Float, 0, LLGL::SystemValue::Color }
    };

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineDesc.renderPass = m_bloom_render_pass;
    pipelineDesc.vertexShader = FullscreenTriangleVertexShader();
    pipelineDesc.indexFormat = LLGL::Format::Undefined;
    pipelineDesc.depth.testEnabled = false;
    pipelineDesc.depth.writeEnabled = false;

    // Prefilter Pipeline
    ShaderSourceCode shaderSource = GetBloomPrefilterShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    m_bloom_prefilter_pipeline = m_context->CreatePipelineState(pipelineDesc);

    // Downsample Pipeline
    shaderSource = GetBloomDownsampleShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    m_bloom_downsample_pipeline = m_context->CreatePipelineState(pipelineDesc);

    // Upsample Pipeline
    shaderSource = GetBloomUpsampleShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    pipelineDesc.blend.targets[0] = LLGL::BlendTargetDescriptor {
        .blendEnabled = true,
        .srcColor = LLGL::BlendOp::One,
        .dstColor = LLGL::BlendOp::One,
        .srcAlpha = LLGL::BlendOp::One,
        .dstAlpha = LLGL::BlendOp::One,
    };
    m_bloom_upsample_pipeline = m_context->CreatePipelineState(pipelineDesc);

    // Composite Pipeline
    shaderSource = GetBloomCompositeShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    pipelineDesc.blend.targets[0] = LLGL::BlendTargetDescriptor {
        .blendEnabled = true,
        .srcColor = LLGL::BlendOp::One,
        .dstColor = LLGL::BlendOp::One,
        .srcAlpha = LLGL::BlendOp::One,
        .dstAlpha = LLGL::BlendOp::One,
    };
    m_bloom_composite_pipeline = m_context->CreatePipelineState(pipelineDesc);
}

void sge::Renderer::BloomPass(sge::Framebuffer& framebuffer, const sge::BloomSettings& settings) {
    if (settings.maxIterations <= 0)
        return;

    if (m_prev_bloom_settings != settings) {
        m_prev_bloom_settings = settings;

        auto uniforms = BloomUniforms {
            .threshold = settings.threshold,
            .knee = settings.knee,
            .filterRadius = settings.scatter,
            .intensity = settings.intensity
        };

        m_command_buffer->UpdateBuffer(*m_bloom_cb, 0, &uniforms, sizeof(uniforms));
    }

    if (!m_bloom_prefilter_pipeline)
        InitBloomPipelines();

    LLGL::Extent2D resolution = framebuffer.GetResolution();
    resolution.width /= 2;
    resolution.height /= 2;

    for (uint8_t i = 0; i < settings.maxIterations; ++i) {
        if (resolution.width < 8 || resolution.height < 8) break;

        m_bloom_framebuffers.emplace_back(m_context->GetTemporaryFramebuffer(resolution, LLGL::Format::RG11B10Float));

        resolution.width /= 2;
        resolution.height /= 2;
    }

    auto& target = m_bloom_framebuffers[0];
    
    m_command_buffer->SetViewport(target.GetResolution());
    BeginPass(*target.GetRenderTarget());
    {
        Clear();
        m_command_buffer->SetPipelineState(*m_bloom_prefilter_pipeline);
        m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
        m_command_buffer->SetResource(0, *m_bloom_cb);
        m_command_buffer->SetResource(1, *framebuffer.GetTexture(0));
        m_command_buffer->SetResource(2, *m_context->GetLinearSampler());
        m_command_buffer->Draw(3, 0);
    }
    EndPass();

    // Downsample
    for (uint32_t i = 1; i < m_bloom_framebuffers.size(); ++i) {  
        m_command_buffer->SetViewport(m_bloom_framebuffers[i].GetResolution());
        BeginPass(*m_bloom_framebuffers[i].GetRenderTarget());
        {
            m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
            m_command_buffer->SetPipelineState(*m_bloom_downsample_pipeline);
            m_command_buffer->SetResource(0, *m_bloom_cb);
            m_command_buffer->SetResource(1, *m_bloom_framebuffers[i - 1].GetTexture(0));
            m_command_buffer->SetResource(2, *m_context->GetLinearSampler());
            m_command_buffer->Draw(3, 0);
        }
        EndPass();
    }

    // Upsample
    for (uint32_t i = m_bloom_framebuffers.size() - 1; i --> 0;) {  
        m_command_buffer->SetViewport(m_bloom_framebuffers[i].GetResolution());
        BeginPass(*m_bloom_framebuffers[i].GetRenderTarget());
        {
            m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
            m_command_buffer->SetPipelineState(*m_bloom_upsample_pipeline);
            m_command_buffer->SetResource(0, *m_bloom_cb);
            m_command_buffer->SetResource(1, *m_bloom_framebuffers[i + 1].GetTexture(0));
            m_command_buffer->SetResource(2, *m_context->GetLinearSampler());
            m_command_buffer->Draw(3, 0);
        }
        EndPass();
    }

    m_command_buffer->SetViewport(framebuffer.GetResolution());
    BeginPass(*framebuffer.GetRenderTarget());
    {
        m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
        m_command_buffer->SetPipelineState(*m_bloom_composite_pipeline);
        m_command_buffer->SetResource(0, *m_bloom_cb);
        m_command_buffer->SetResource(1, *m_bloom_framebuffers[0].GetTexture(0));
        m_command_buffer->SetResource(2, *m_context->GetLinearSampler());
        m_command_buffer->Draw(3, 0);
    }
    EndPass();

    m_bloom_framebuffers.clear();
}

void sge::Renderer::BlitTexture(LLGL::Texture& texture) {
    LLGL::RenderTarget* target = GetRenderContext()->GetCurrentTarget();
    SGE_ASSERT(target != nullptr);

    m_command_buffer->SetViewport(target->GetResolution());
    m_command_buffer->SetPipelineState(*m_blit_pipeline);
    m_command_buffer->SetVertexBuffer(*FullscreenTriangleVertexBuffer());
    m_command_buffer->SetResource(0, texture);
    m_command_buffer->SetResource(1, *m_context->GetNearestSampler());
    m_command_buffer->Draw(3, 0);
}