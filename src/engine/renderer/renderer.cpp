#include <SGE/assert.hpp>
#include <SGE/log.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/attributes.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/framebuffer.hpp>
#include <SGE/utils/alloc.hpp>
#include <SGE/utils/hash.hpp>

#include <LLGL/CommandBufferFlags.h>
#include <LLGL/Format.h>
#include <LLGL/PipelineCache.h>
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/Texture.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Types.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/VertexAttribute.h>

#include "shaders.hpp"
#include "utils.hpp"

#if SGE_IMGUI_ENABLED
    #include <imgui.h>
    #include "imgui_renderer.hpp"
#endif

namespace {

struct alignas(16) BloomUniforms {
    float threshold;
    float knee;
    float filterRadius;
    float intensity;
};

struct alignas(16) TonemapUniforms {
    float exposure;
};

constexpr LLGL::Format HDR_TEXTURE_FORMAT = LLGL::Format::RG11B10Float;

} // namespace

sge::Renderer::Renderer(const std::shared_ptr<RenderContext>& context) : m_context(context) {
    SGE_ASSERT(context->GetLLGLContext() != nullptr);

    const RenderBackend backend = context->Backend();

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = m_context->GetCommandBuffer();
    m_command_queue = m_context->GetCommandQueue();

    m_uniform_buffer = m_context->CreateConstantBuffer(sizeof(GlobalUniforms), "Uniforms Buffer");

    const glm::vec2 vertices[] = {
        glm::vec2(-1.0f, 1.0f),  glm::vec2(0.0f, 0.0f),
        glm::vec2(3.0f,  1.0f),  glm::vec2(2.0f, 0.0f),
        glm::vec2(-1.0f, -3.0f), glm::vec2(0.0f, 2.0f),
    };

    m_fullscreen_triangle_mesh = CreateMesh(MeshDesc()
        .SetTopology(sge::PrimitiveTopology::TriangleList)
        .SetFrontFace(sge::FrontFace::CW)
        .AddAttribute(sge::VertexFormat::Float32x2, "a_position", "Position")
        .AddAttribute(sge::VertexFormat::Float32x2, "a_uv", "UV")
        .SetVertices(vertices)
    );

    ShaderSourceCode shader = GetFullscreenTriangleShaderSourceCode(backend);
    m_fullscreen_triangle_vertex_shader = context->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size);
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

    LLGL::GraphicsPipelineDescriptor pipelineDesc = GraphicsPipelineDescFromMesh(*m_fullscreen_triangle_mesh);
    pipelineDesc.pipelineLayout = m_blit_pipeline_layout;
    pipelineDesc.vertexShader = m_fullscreen_triangle_vertex_shader;
    pipelineDesc.fragmentShader = m_blit_pixel_shader;
    pipelineDesc.renderPass = m_blit_render_pass;
    pipelineDesc.depth.testEnabled = false;
    pipelineDesc.depth.writeEnabled = false;
    m_blit_pipeline = m_context->CreatePipelineState(pipelineDesc);
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

void sge::Renderer::InitTonemapPipeline(sge::Tonemapping method) {
    if (!m_tonemap_cb) {
        m_tonemap_cb = m_context->CreateConstantBuffer(sizeof(TonemapUniforms), "TonemapConstantBuffer");
    }

    if (!m_tonemap_pipeline_layout) {
        LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindings = sge::BindingLayout({
            sge::BindingLayoutItem::ConstantBuffer(3, "TonemapConstantBuffer", LLGL::StageFlags::FragmentStage),
            sge::BindingLayoutItem::Texture(4, "MainTexture", LLGL::StageFlags::FragmentStage),
            sge::BindingLayoutItem::Sampler(5, "MainSampler", LLGL::StageFlags::FragmentStage),
        });

        m_tonemap_pipeline_layout =  m_context->CreatePipelineLayout(pipelineLayoutDesc);;
    }

    if (!m_tonemap_render_pass) {
        LLGL::RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachments[0].format = HDR_TEXTURE_FORMAT;
        renderPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
        m_tonemap_render_pass = m_context->CreateRenderPass(renderPassDesc);
    }

    ShaderSourceCode shaderSource;

    switch (method) {
    case Tonemapping::AcesFit:
        shaderSource = GetTonemapAcesFittedShaderSourceCode(m_context->Backend());
    break;
    case Tonemapping::Aces:
        shaderSource = GetTonemapAcesShaderSourceCode(m_context->Backend());
    break;
    default:
        SGE_UNREACHABLE();
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc = GraphicsPipelineDescFromMesh(*m_fullscreen_triangle_mesh);
    pipelineDesc.pipelineLayout = m_tonemap_pipeline_layout;
    pipelineDesc.renderPass = m_tonemap_render_pass;
    pipelineDesc.vertexShader = FullscreenTriangleVertexShader();
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size);
    pipelineDesc.depth.testEnabled = false;
    pipelineDesc.depth.writeEnabled = false;

    m_tonemap_pipelines[static_cast<uint8_t>(method)] = m_context->CreatePipelineState(pipelineDesc);
}

void sge::Renderer::TonemapPass(sge::Framebuffer& framebuffer, const sge::TonemapSettings& settings) {
    const auto& pipeline = m_tonemap_pipelines[static_cast<uint8_t>(settings.method)];

    if (!pipeline)
        InitTonemapPipeline(settings.method);

    if (m_prev_tonemap_settings != settings) {
        m_prev_tonemap_settings = settings;

        auto uniforms = TonemapUniforms {
            .exposure = glm::exp2(settings.exposure)
        };

        m_command_buffer->UpdateBuffer(*m_tonemap_cb, 0, &uniforms, sizeof(uniforms));
    }

    LLGL::Extent2D resolution = framebuffer.GetResolution();

    m_command_buffer->SetViewport(resolution);
    BeginPass(*framebuffer.GetRenderTarget());
    {
        m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
        m_command_buffer->SetPipelineState(*pipeline);
        m_command_buffer->SetResource(0, *m_tonemap_cb);
        m_command_buffer->SetResource(1, *framebuffer.GetTexture(0));
        m_command_buffer->SetResource(2, *m_context->GetLinearSampler());
        m_command_buffer->Draw(3, 0);
    }
    EndPass();
}

void sge::Renderer::InitBloomPipelines() {
    m_bloom_cb = m_context->CreateConstantBuffer(sizeof(BloomUniforms), "BloomConstantBuffer");

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(3, "BloomConstantBuffer", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Texture(4, "MainTexture", LLGL::StageFlags::FragmentStage),
        sge::BindingLayoutItem::Sampler(5, "MainSampler", LLGL::StageFlags::FragmentStage),
    });

    LLGL::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachments[0].format = HDR_TEXTURE_FORMAT;
    renderPassDesc.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    m_bloom_render_pass = m_context->CreateRenderPass(renderPassDesc);

    sge::ShaderConfig shaderConfig;
    shaderConfig.fragment.outputAttribs = {
        LLGL::FragmentAttribute{ "SV_Target", HDR_TEXTURE_FORMAT, 0, LLGL::SystemValue::Color }
    };

    LLGL::GraphicsPipelineDescriptor pipelineDesc = GraphicsPipelineDescFromMesh(*m_fullscreen_triangle_mesh);
    pipelineDesc.pipelineLayout = m_context->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineDesc.renderPass = m_bloom_render_pass;
    pipelineDesc.vertexShader = FullscreenTriangleVertexShader();
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

    pipelineDesc.blend.targets[0] = LLGL::BlendTargetDescriptor {
        .blendEnabled = true,
        .srcColor = LLGL::BlendOp::One,
        .dstColor = LLGL::BlendOp::One,
        .srcAlpha = LLGL::BlendOp::One,
        .dstAlpha = LLGL::BlendOp::Zero,
    };

    // Upsample Pipeline
    shaderSource = GetBloomUpsampleShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    m_bloom_upsample_pipeline = m_context->CreatePipelineState(pipelineDesc);

    // Composite Pipeline
    shaderSource = GetBloomCompositeShaderSourceCode(m_context->Backend());
    pipelineDesc.fragmentShader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shaderSource.fs_source, shaderSource.fs_size, shaderConfig);
    m_bloom_composite_pipeline = m_context->CreatePipelineState(pipelineDesc);
}

void sge::Renderer::BloomPass(sge::Framebuffer& framebuffer, const sge::BloomSettings& settings) {
    if (settings.maxIterations <= 0)
        return;

    if (!m_bloom_prefilter_pipeline)
        InitBloomPipelines();

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

    LLGL::Extent2D resolution = framebuffer.GetResolution();
    resolution.width /= 2;
    resolution.height /= 2;

    for (uint8_t i = 0; i < settings.maxIterations; ++i) {
        if (resolution.width < 8 || resolution.height < 8) break;

        m_bloom_framebuffers.emplace_back(m_context->GetTemporaryFramebuffer(resolution, HDR_TEXTURE_FORMAT));

        resolution.width /= 2;
        resolution.height /= 2;
    }

    auto& target = m_bloom_framebuffers[0];

    m_command_buffer->SetViewport(target.GetResolution());
    BeginPass(*target.GetRenderTarget());
    {
        Clear();
        m_command_buffer->SetPipelineState(*m_bloom_prefilter_pipeline);
        m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
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
            m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
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
            m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
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
        m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
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
    m_command_buffer->SetVertexBuffer(*m_fullscreen_triangle_mesh->GetVertexBuffer());
    m_command_buffer->SetResource(0, texture);
    m_command_buffer->SetResource(1, *m_context->GetNearestSampler());
    m_command_buffer->Draw(3, 0);
}

LLGL::PipelineState& sge::Renderer::GetOrCreatePipeline(const Material& material, const Mesh& mesh) {
    ZoneScoped;
    auto materialHash = material.GetStateHash();
    auto meshHash = mesh.GetLayoutHash();

    auto key = PipelineKey { .meshHash = meshHash, .materialHash = materialHash };

    auto existingEntry = m_pipelines.find(key);
    if (existingEntry != m_pipelines.end()) {
        return m_context->GetOrCreatePipeline(existingEntry->second);
    }

    std::vector<LLGL::VertexAttribute> totalAttributes = mesh.GetVertexAttributes();
    for (const LLGL::VertexAttribute& attribute : material.GetInstanceAttribs()) {
        uint32_t prevLocation = totalAttributes.empty() ? 0 : totalAttributes.back().location;
        totalAttributes.push_back(attribute);
        totalAttributes.back().location = prevLocation + 1;
    }

    sge::GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.layout = material.GetPipelineLayout();
    pipelineConfig.vertexShader = material.GetVertexShader();
    pipelineConfig.pixelShader = material.GetFragmentShader();
    pipelineConfig.inputVertexAttribs = std::move(totalAttributes);
    pipelineConfig.indexFormat = mesh.GetIndexFormat();
    pipelineConfig.primitiveTopology = mesh.GetTopology();
    pipelineConfig.frontFace = mesh.GetFrontFace();
    pipelineConfig.cullMode = material.GetCullMode();
    pipelineConfig.blend.targets[0] = ConvertBlendModeToLLGL(material.GetBlendMode());
    pipelineConfig.scissorTestEnabled = material.IsScissorTestEnabled();

    if (material.IsDepthTestEnabled()) {
        pipelineConfig.depth.testEnabled = true;
        pipelineConfig.depth.writeEnabled = true;
        pipelineConfig.depth.compareOp = LLGL::CompareOp::GreaterEqual;
    }

    sge::PipelineId pipeline = m_context->CreateGraphicsPipeline(pipelineConfig);
    auto [it, success] = m_pipelines.try_emplace(key, pipeline);
    SGE_ASSERT(success);

    return m_context->GetOrCreatePipeline(pipeline);
}

sge::Ref<sge::Mesh> sge::Renderer::GetDefaultBatch2dMesh() {
    if (!m_2d_batch_mesh) {
        glm::vec2 vertices[] = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };

        m_2d_batch_mesh = CreateMesh(sge::MeshDesc()
            .AddAttribute(sge::VertexFormat::Float32x2, "inp_position", "Position")
            .SetTopology(sge::PrimitiveTopology::TriangleStrip)
            .SetFrontFace(sge::FrontFace::CCW)
            .SetVertices(vertices)
        );
    }
    return m_2d_batch_mesh;
}

template <typename TKey, typename TValue, typename MapKeyHasher>
static inline TValue& MapFind(const TKey& key, std::unordered_map<TKey, size_t, MapKeyHasher>& map, std::vector<TValue>& pool) {
    size_t index = pool.size();
    auto it = map.find(key);
    if (it != map.end()) {
        index = it->second;
    } else {
        map.emplace(key, index);
        pool.emplace_back();
    }

    return pool[index];
}

void sge::Renderer::SubmitManyRaw(
    Ref<Mesh> mesh,
    Ref<Material> material,
    std::span<LLGL::Resource* const> dynamicBindings,
    sge::IRect scissorBounds,
    const void* instanceDataPtr,
    size_t instanceStride,
    uint32_t instanceCount
) {
    ZoneScoped;

    if (instanceCount == 0)
        return;

    auto materialHash = material->GetStateHash();
    auto meshHash = mesh->GetLayoutHash();
    auto pipelineKey = PipelineKey { .meshHash = meshHash, .materialHash = materialHash };

    auto& instanceData = MapFind(pipelineKey, m_instance_data_map, m_instance_datas);

    if (!instanceData.mesh)
        instanceData.mesh = mesh;

    uint64_t dynamicBindingsHash = sge::HASH_INIT;
    sge::hash_fnv1a(dynamicBindingsHash, dynamicBindings.data(), dynamicBindings.size());

    uint64_t scissorHash = sge::HASH_INIT;
    sge::hash_fnv1a(scissorHash, &scissorBounds);

    BatchKey batchKey {
        .mesh = std::move(mesh),
        .material = std::move(material),
        .dynamicBindingsHash = dynamicBindingsHash,
        .scissorHash = scissorHash
    };

    auto& batch = m_mesh_batches[batchKey];

    if (!batch.mesh)
        batch.mesh = std::move(mesh);

    if (!batch.material)
        batch.material = std::move(material);

    if (batch.dynamicBindings.empty() && !dynamicBindings.empty()) {
        batch.dynamicBindings.insert(batch.dynamicBindings.end(), dynamicBindings.begin(), dynamicBindings.end());
    }

    const size_t instanceByteSize = instanceCount * instanceStride;

    size_t instanceOffset = static_cast<size_t>(-1);

    if (instanceDataPtr != nullptr && instanceByteSize > 0) {
        instanceOffset = instanceData.totalInstanceCount;
        if (instanceData.instanceBufferPool.GetStride() == 0) {
            LLGL::BufferDescriptor desc = LLGL::VertexBufferDesc(0, batch.material->GetInstanceAttribsStride());
            instanceData.instanceBufferPool = BufferPool(*m_context, desc);
        }
        const auto* bytes = static_cast<const uint8_t*>(instanceDataPtr);
        instanceData.instanceBytes.insert(instanceData.instanceBytes.end(), bytes, bytes + instanceByteSize);

        instanceData.totalInstanceCount += instanceCount;
    }

    if (!m_batch_submissions.empty()) {
        MeshBatchSubmission& submission = m_batch_submissions.back();
        if (submission.batch == &batch) {
            submission.instanceCount += instanceCount;
            return;
        }
    }

    m_batch_submissions.push_back(MeshBatchSubmission {
        .batch = &batch,
        .instanceOffset = instanceOffset,
        .instanceCount = instanceCount,
    });
}

void sge::Renderer::FlushBatchRawImpl(MeshBatch& batch, size_t instanceOffset, size_t instanceCount) {
    ZoneScoped;

    Mesh& mesh = *batch.mesh;
    Material& material = *batch.material;

    m_command_buffer->SetPipelineState(GetOrCreatePipeline(material, mesh));

    auto pipelineKey = PipelineKey { .meshHash = mesh.GetLayoutHash(), .materialHash = material.GetStateHash() };

    InstanceData* instanceData = nullptr;
    auto it = m_instance_data_map.find(pipelineKey);
    if (it != m_instance_data_map.end()) {
        instanceData = &m_instance_datas[it->second];
    }

    const bool hasInstanceBuffer = instanceData != nullptr && instanceOffset < instanceData->totalInstanceCount;

    if (hasInstanceBuffer) {
        m_command_buffer->SetVertexBufferArray(*instanceData->vertexBufferArray);
    } else {
        m_command_buffer->SetVertexBuffer(*mesh.GetVertexBuffer());
        instanceOffset = 0;
    }

    m_command_buffer->SetResourceHeap(*material.GetResourceHeap());

    for (size_t i = 0; i < batch.dynamicBindings.size(); ++i) {
        m_command_buffer->SetResource(i, *batch.dynamicBindings[i]);
    }

    if (batch.material->IsScissorTestEnabled()) {
        sge::IRect scissor = batch.scissorBounds;

        if (scissor.width() <= 0 || scissor.height() <= 0) {
            // Reset scissor
            LLGL::Extent2D resolution = m_context->GetCurrentTarget()->GetResolution();
            m_command_buffer->SetScissor(LLGL::Scissor(0, 0, resolution.width, resolution.height));
        } else {
            m_command_buffer->SetScissor(LLGL::Scissor(scissor.min.x, scissor.min.y, scissor.max.x, scissor.max.y));
        }
    }

    auto indexCount = mesh.GetIndexCount();

    if (indexCount > 0) {
        m_command_buffer->SetIndexBuffer(*mesh.GetIndexBuffer());
        m_command_buffer->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, instanceCount);
    } else {
        auto vertexCount = mesh.GetVertexCount();
        m_command_buffer->DrawInstanced(vertexCount, 0, instanceCount, instanceOffset);
    }
}

void sge::Renderer::UploadBatchBuffers() {
    ZoneScoped;

    for (auto& instanceData : m_instance_datas) {
        if (instanceData.instanceBytes.empty())
            continue;

        const size_t instanceDataSize = instanceData.instanceBytes.size();

        if (instanceData.instanceBufferPool.Reserve(*m_context, instanceDataSize)) {
            SGE_ASSERT(instanceData.instanceBufferPool.Get() != nullptr);
            if (instanceData.vertexBufferArray)
                m_context->Release(*instanceData.vertexBufferArray);
            instanceData.vertexBufferArray = m_context->CreateBufferArray({ instanceData.mesh->GetVertexBuffer().Get(), instanceData.instanceBufferPool.Get() });
        }
        m_command_buffer->UpdateBuffer(*instanceData.instanceBufferPool.Get(), 0, instanceData.instanceBytes.data(), instanceDataSize);

        instanceData.instanceBytes.clear();
    }
}

void sge::Renderer::EndPass() {
    ZoneScoped;

    UploadBatchBuffers();

    for (auto& submission : m_batch_submissions) {
        MeshBatch& batch = *submission.batch;

        if (submission.instanceCount == 0)
            continue;

        FlushBatchRawImpl(batch, submission.instanceOffset, submission.instanceCount);
        submission.instanceCount = 0;
        submission.instanceOffset = static_cast<size_t>(-1);
    }

    for (auto& instanceData : m_instance_datas) {
        instanceData.totalInstanceCount = 0;
    }

    m_batch_submissions.clear();

#if SGE_IMGUI_ENABLED
    if (ImGuiRenderer::IsActive()) {
        auto* drawData = ImGui::GetDrawData();
        if (drawData != nullptr) {
            ImGuiRenderer::RenderDrawData(drawData);
        }
    }
#endif

    m_context->PopRenderTarget();
    m_command_buffer->EndRenderPass();
}