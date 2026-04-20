#include "LLGL/Constants.h"
#include <algorithm>
#include <filesystem>

#include <LLGL/ResourceFlags.h>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/profile.hpp>

namespace fs = std::filesystem;

bool sge::RenderContext::Init(sge::RenderBackend backend) {
    m_backend = backend;

    LLGL::Report report;

    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName = backend.ToString();

    if (backend.IsOpenGL()) {
        LLGL::RendererConfigurationOpenGL* config = new LLGL::RendererConfigurationOpenGL();
        config->contextProfile = LLGL::OpenGLContextProfile::CoreProfile;

        rendererDesc.rendererConfig = config;
        rendererDesc.rendererConfigSize = sizeof(LLGL::RendererConfigurationOpenGL);
    }

#if SGE_DEBUG
    m_debugger = new LLGL::RenderingDebugger();
    rendererDesc.flags    = LLGL::RenderSystemFlags::DebugDevice;
    rendererDesc.debugger = m_debugger;
#endif

    m_context = LLGL::RenderSystem::Load(rendererDesc, &report);

    if (backend.IsOpenGL()) {
        delete (LLGL::OpenGLContextProfile*) rendererDesc.rendererConfig;
    }

    if (report.HasErrors()) {
        SGE_LOG_ERROR("An error occured while loading render system: {}", report.GetText());
        return false;
    }

    if (m_context == nullptr) {
        SGE_LOG_ERROR("Couldn't load render system");
        return false;
    }

    const LLGL::RendererInfo& info = GetRendererInfo();

    SGE_LOG_INFO("Renderer:             {}", info.rendererName.c_str());
    SGE_LOG_INFO("Device:               {}", info.deviceName.c_str());
    SGE_LOG_INFO("Vendor:               {}", info.vendorName.c_str());
    SGE_LOG_INFO("Shading Language:     {}", info.shadingLanguageName.c_str());

    SGE_LOG_INFO("Extensions:");
    for (const auto& extension : info.extensionNames) {
        SGE_LOG_INFO("  {}", extension.c_str());
    }

    return true;
}

void sge::RenderContext::Destroy() {
    m_context->GetCommandQueue()->WaitIdle();

    m_pipeline_configs.clear();
    m_render_target_configs.clear();
    m_render_pass_configs.clear();

    for (auto& [key, swapChain] : m_swapchain_map) {
        Release(*swapChain);
    }

    for (auto& [key, renderPass] : m_render_passes) {
        Release(*renderPass);
    }

    for (auto& [key, renderTarget] : m_render_targets) {
        Release(*renderTarget);
    }

    for (auto& [key, pipelineState] : m_pipeline_states) {
        Release(*pipelineState);
    }
}

sge::RenderContext::~RenderContext() {
    m_context->GetCommandQueue()->WaitIdle();

#if SGE_DEBUG
    if (m_debugger != nullptr) {
        delete m_debugger;
    }
#endif

    if (m_context != nullptr) {
        LLGL::RenderSystem::Unload(std::move(m_context));
    }
}

LLGL::PipelineCache* sge::RenderContext::ReadPipelineCache(const std::filesystem::path& dir, const std::string& name, bool& hasInitialCache) {
    // Try to read PSO cache from file
    const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
    const fs::path cachePath = dir / cacheFilename;

    LLGL::Blob pipelineCacheBlob = LLGL::Blob::CreateFromFile(cachePath.string());
    if (pipelineCacheBlob) {
        hasInitialCache = true;
    }

    return m_context->CreatePipelineCache(pipelineCacheBlob);
}

void sge::RenderContext::SavePipelineCache(const std::filesystem::path& dir, const std::string& name, LLGL::PipelineCache& pipelineCache) {
    if (LLGL::Blob psoCache = pipelineCache.GetBlob())
    {
        const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
        const fs::path cachePath = dir / cacheFilename;

        // Store PSO cache to file
        std::ofstream file{ cachePath, std::ios::out | std::ios::binary };
        file.write(
            reinterpret_cast<const char*>(psoCache.GetData()),
            static_cast<std::streamsize>(psoCache.GetSize())
        );
    }
}

void sge::RenderContext::UnregisterWindow(const GlfwWindow& window) {
    auto it = m_swapchain_map.find(window.GetID());
    if (it == m_swapchain_map.end())
        return;

    if (m_context != nullptr)
        Release(*it->second);

    m_swapchain_map.erase(window.GetID());
}

LLGL::SwapChain& sge::RenderContext::GetOrCreateSwapChain(const std::shared_ptr<GlfwWindow>& window) {
    LLGL::SwapChain* swap_chain = nullptr;

    auto it = m_swapchain_map.find(window->GetID());

    if (it == m_swapchain_map.end()) {
        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = window->GetContentSize();
        swapChainDesc.fullscreen = window->IsFullscreen();
        swapChainDesc.samples = window->GetSamples();
        // swapChainDesc.colorBits = settings.transparent ? 32 : 24;

        swap_chain = m_context->CreateSwapChain(swapChainDesc, window);
        SGE_ASSERT(swap_chain != nullptr);

        swap_chain->SetVsyncInterval(window->GetVsyncInterval());

        m_swapchain_map.try_emplace(window->GetID(), swap_chain);
    } else {
        swap_chain = it->second;
    }

    return *swap_chain;
}

LLGL::SwapChain* sge::RenderContext::GetSwapChain(const std::shared_ptr<GlfwWindow>& window) {
    auto it = m_swapchain_map.find(window->GetID());
    if (it == m_swapchain_map.end()) {
        return nullptr;
    }
    return it->second;
}

void sge::RenderContext::Present(const std::shared_ptr<sge::GlfwWindow>& window) {
    auto it = m_swapchain_map.find(window->GetID());
    SGE_ASSERT(it != m_swapchain_map.end());

    it->second->Present();
}

LLGL::PipelineState& sge::RenderContext::GetOrCreatePipeline(Handle<LLGL::PipelineState> handle) {
    SGE_ASSERT(m_current_target != nullptr);

    const auto it_config = m_pipeline_configs.find(handle.ID());
    SGE_ASSERT(it_config != m_pipeline_configs.end());

    const GraphicsPipelineConfig& config = it_config->second;

    LLGL::PipelineState* pipeline_state = nullptr;

    const PipelineConfigKey key = {.render_target=m_current_target, .config_id=handle.ID()};
    const auto it_pipeline = m_pipeline_states.find(key);
    if (it_pipeline != m_pipeline_states.end()) {
        pipeline_state = it_pipeline->second;
    } else {
        LLGL::RenderPass* renderPass = nullptr;
        if (config.renderPass.IsValid()) {
            renderPass = &GetOrCreateRenderPass(config.renderPass, m_current_target->GetSamples());
        } 

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.depth = config.depth;
        pipelineDesc.blend = config.blend;
        pipelineDesc.debugName = config.debugName.c_str();
        pipelineDesc.pipelineLayout = config.layout.Get();
        pipelineDesc.vertexShader = config.vertexShader.Get();
        pipelineDesc.geometryShader = config.geometryShader.Get();
        pipelineDesc.fragmentShader = config.pixelShader.Get();
        pipelineDesc.primitiveTopology = config.primitiveTopology;
        pipelineDesc.indexFormat = config.indexFormat;
        pipelineDesc.renderPass = renderPass;
        pipelineDesc.rasterizer.cullMode = config.cullMode;
        pipelineDesc.rasterizer.frontCCW = config.frontCCW;
        pipelineDesc.rasterizer.scissorTestEnabled = config.scissorTestEnabled;
        pipelineDesc.rasterizer.multiSampleEnabled = (m_current_target->GetSamples() > 1);
        
        if (pipelineDesc.renderPass == nullptr) {
            pipelineDesc.renderPass = m_current_target->GetRenderPass();
        }

        pipeline_state = m_context->CreatePipelineState(pipelineDesc);
        SGE_ASSERT(pipeline_state != nullptr);

        m_pipeline_states[key] = pipeline_state;
    }

    return *pipeline_state;
}

LLGL::RenderTarget& sge::RenderContext::GetOrCreateRenderTarget(Handle<LLGL::RenderTarget> handle, uint8_t samples) {
    const auto it_config = m_render_target_configs.find(handle.ID());
    SGE_ASSERT(it_config != m_render_target_configs.end());

    const RenderTargetConfig& config = it_config->second;

    LLGL::RenderTarget* renderTarget = nullptr;

    const RenderTargetKey key = {
        .resolution=config.resolution,
        .config_id=handle.ID(),
        .render_pass_id=config.renderPass.ID(),
        .format=config.format,
        .samples=samples,
    };

    const auto it_target = m_render_targets.find(key);
    if (it_target != m_render_targets.end()) {
        renderTarget = it_target->second;
    } else {
        LLGL::RenderPass* renderPass = nullptr;
        if (config.renderPass.IsValid()) {
            renderPass = &GetOrCreateRenderPass(config.renderPass, samples);
        }

        LLGL::RenderTargetDescriptor targetDesc;
        targetDesc.resolution = config.resolution;
        targetDesc.samples = samples;
        targetDesc.renderPass = renderPass;

        if (samples > 1) {
            std::ranges::copy(config.colorAttachments, targetDesc.resolveAttachments);

            LLGL::TextureDescriptor textureDesc;
            textureDesc.type = LLGL::TextureType::Texture2DMS;
            textureDesc.extent.width = config.resolution.width;
            textureDesc.extent.height = config.resolution.height;
            textureDesc.miscFlags = LLGL::MiscFlags::FixedSamples;
            textureDesc.cpuAccessFlags = 0;
            textureDesc.samples = samples;
            
            for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
                LLGL::Format attachmentFormat = config.colorAttachments[i].format;
                LLGL::Texture* attachmentTexture = config.colorAttachments[i].texture;

                if (attachmentFormat == LLGL::Format::Undefined && attachmentTexture != nullptr) {
                    attachmentFormat = attachmentTexture->GetFormat();
                }
                
                if (attachmentFormat != LLGL::Format::Undefined) {
                    textureDesc.format = attachmentFormat;
                    targetDesc.colorAttachments[i].texture = m_context->CreateTexture(textureDesc);
                }
            }
        } else {
            for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
                targetDesc.colorAttachments[i] = config.colorAttachments[i];
            }
        }

        targetDesc.depthStencilAttachment = config.depthStencilAttachment;

        renderTarget = m_context->CreateRenderTarget(targetDesc);

        m_render_targets.try_emplace(key, renderTarget);
    }

    return *renderTarget;
}

LLGL::RenderPass& sge::RenderContext::GetOrCreateRenderPass(Handle<LLGL::RenderPass> handle, uint8_t samples) {
    const auto it_config = m_render_pass_configs.find(handle.ID());
    SGE_ASSERT(it_config != m_render_pass_configs.end());

    const RenderPassConfig& config = it_config->second;

    LLGL::RenderPass* renderPass = nullptr;

    const RenderPassKey key = {
        .config_id=handle.ID(),
        .samples=samples,
    };

    const auto it_target = m_render_passes.find(key);
    if (it_target != m_render_passes.end()) {
        renderPass = it_target->second;
    } else {
        LLGL::RenderPassDescriptor targetDesc;
        targetDesc.depthAttachment = config.depthAttachment;
        targetDesc.stencilAttachment = config.stencilAttachment;
        targetDesc.samples = samples;

        for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
            targetDesc.colorAttachments[i] = config.colorAttachments[i];
        }

        renderPass = m_context->CreateRenderPass(targetDesc);

        m_render_passes.try_emplace(key, renderPass);
    }

    return *renderPass;
}

void sge::RenderContext::DeletePipeline(Handle<LLGL::PipelineState> handle) {
    if (!handle.IsValid()) return;

    for (auto it = m_pipeline_states.begin(); it != m_pipeline_states.end(); ++it) {
        auto key = it->first;
        auto value = it->second;

        if (key.config_id == handle.ID()) {
            Release(*value);
            m_pipeline_states.erase(it);
            break;
        }
    }
}

void sge::RenderContext::DeleteRenderTarget(Handle<LLGL::RenderTarget> handle) {
    if (!handle.IsValid()) return;

    for (auto it = m_render_targets.begin(); it != m_render_targets.end(); ++it) {
        auto key = it->first;
        auto value = it->second;

        if (key.config_id == handle.ID()) {
            Release(*value);
            m_render_targets.erase(it);
            break;
        }
    }
}

sge::Raw<sge::Sampler> sge::RenderContext::CreateSampler(const LLGL::SamplerDescriptor& descriptor) {
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    SGE_ASSERT(sampler != nullptr);
    return Raw<sge::Sampler>::Create(new Sampler(CreateUnique(sampler), descriptor));
}

sge::Texture sge::RenderContext::CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const Ref<Sampler>& sampler, const void* data, bool generate_mip_maps) {
    ZoneScoped;

    LLGL::TextureDescriptor texture_desc;
    texture_desc.type = type;
    texture_desc.extent = LLGL::Extent3D(width, height, 1);
    texture_desc.arrayLayers = layers;
    texture_desc.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment;
    texture_desc.cpuAccessFlags = 0;
    texture_desc.miscFlags = LLGL::MiscFlags::GenerateMips * generate_mip_maps;
    texture_desc.mipLevels = generate_mip_maps ? 0 : 1;

    uint32_t components = LLGL::ImageFormatSize(image_format);

    LLGL::ImageView image_view;
    image_view.format = image_format;
    image_view.dataType = data_type;
    image_view.data = data;
    image_view.dataSize = width * height * layers * components;

    uint32_t id = m_texture_index++;

    LLGL::Texture* texture = m_context->CreateTexture(texture_desc, &image_view);
    SGE_ASSERT(texture != nullptr);

    return sge::Texture(id, sge::Size(width, height), sampler, Ref<LLGL::Texture>(shared_from_this(), texture));
}

sge::Raw<LLGL::Shader> sge::RenderContext::LoadShaderFromFile(const ShaderPath& shader_path, const std::vector<ShaderDef>& shader_defs, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
    ZoneScoped;

    const RenderBackend backend = m_backend;
    const ShaderType shader_type = shader_path.shader_type;
    const char* entry_point = shader_type.IsCompute() ? shader_path.func_name.c_str() : shader_type.EntryPoint(backend);

    std::string filename = shader_type.IsCompute() ? shader_path.func_name : shader_path.name;
    if (backend.IsVulkan() || backend.IsOpenGL()) {
        filename += '.';
        filename += shader_type.Stage();
    }

    filename += shader_type.FileExtension(backend);

    const fs::path path = fs::path(backend.AssetFolder()) / filename;
    const std::string path_str = path.string().c_str();

    if (!fs::exists(path)) {
        SGE_LOG_ERROR("Failed to find shader '{}'", path_str.c_str());
        return sge::Raw<LLGL::Shader>::Create(shared_from_this(), nullptr);
    }

    uint8_t* data = nullptr;
    uintmax_t data_length = 0;
    
    if (backend.IsVulkan()) {
        std::ifstream shader_file(path, std::ios::binary);
        data_length = fs::file_size(path);
        data = new uint8_t[data_length];
        shader_file.read(reinterpret_cast<char*>(data), data_length);
    } else {
        std::ifstream shader_file(path);
        std::stringstream buffer;
        buffer << shader_file.rdbuf();
        std::string shader_source = buffer.str();

        for (const ShaderDef& shader_def : shader_defs) {
            size_t pos;
            while ((pos = shader_source.find(shader_def.name)) != std::string::npos) {
                shader_source.replace(pos, shader_def.name.length(), shader_def.value);
            }
        }

        data_length = shader_source.size() + 1;
        data = new uint8_t[data_length];
        data[data_length - 1] = '\0';
        memcpy(data, shader_source.data(), data_length - 1);
    }

    LLGL::Shader* shader = CreateShader(shader_path.shader_type, entry_point, data, data_length, vertex_attributes);
    delete[] data;

    if (!shader) {
        fmt::println(stderr, "Shader file: {}", path_str);
    }
    return sge::Raw<LLGL::Shader>::Create(shared_from_this(), shader);
}

sge::Raw<LLGL::Shader> sge::RenderContext::CreateShader(sge::ShaderType shader_type, const char* entry_point, const void* data, size_t length, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
    const RenderBackend backend = m_backend;

    LLGL::ShaderDescriptor shader_desc;
    shader_desc.type = shader_type.ToLLGLType();
    shader_desc.sourceType = LLGL::ShaderSourceType::CodeString;
    shader_desc.source = static_cast<const char*>(data);
    shader_desc.sourceSize = length;

    if (backend.IsVulkan()) {
        shader_desc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
    }

    if (!backend.IsVulkan() && !backend.IsOpenGL()) {
        shader_desc.entryPoint = entry_point;
        shader_desc.profile = shader_type.Profile(backend);
    }

    if (shader_type.IsVertex()) {
        shader_desc.vertex.inputAttribs = vertex_attributes;
    }

    if (backend.IsOpenGL() && shader_type.IsFragment()) {
        shader_desc.fragment.outputAttribs = {
            { "fragColor", LLGL::Format::RGBA8UNorm, 0, LLGL::SystemValue::Color }
        };
    }

#if SGE_DEBUG
    shader_desc.flags |= LLGL::ShaderCompileFlags::NoOptimization;
#else
    shader_desc.flags |= LLGL::ShaderCompileFlags::OptimizationLevel3;
#endif

    LLGL::Shader* shader = m_context->CreateShader(shader_desc);
    if (const LLGL::Report* report = shader->GetReport()) {
        if (*report->GetText() != '\0') {
            if (report->HasErrors()) {
                SGE_LOG_ERROR("Failed to create a shader. Error: {}", report->GetText());
                return sge::Raw<LLGL::Shader>::Create(shared_from_this(), nullptr);
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    return sge::Raw<LLGL::Shader>::Create(shared_from_this(), shader);
}

void sge::RenderContext::ReleaseUntyped(LLGL::RenderSystemChild& resource) {
    if (LLGL::Buffer* r = LLGL::CastTo<LLGL::Buffer>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::BufferArray* r = LLGL::CastTo<LLGL::BufferArray>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::Texture* r = LLGL::CastTo<LLGL::Texture>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::Sampler* r = LLGL::CastTo<LLGL::Sampler>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::Shader* r = LLGL::CastTo<LLGL::Shader>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::PipelineLayout* r = LLGL::CastTo<LLGL::PipelineLayout>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::PipelineState* r = LLGL::CastTo<LLGL::PipelineState>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::RenderPass* r = LLGL::CastTo<LLGL::RenderPass>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::ResourceHeap* r = LLGL::CastTo<LLGL::ResourceHeap>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::PipelineCache* r = LLGL::CastTo<LLGL::PipelineCache>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::QueryHeap* r = LLGL::CastTo<LLGL::QueryHeap>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::Fence* r = LLGL::CastTo<LLGL::Fence>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::SwapChain* r = LLGL::CastTo<LLGL::SwapChain>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::RenderTarget* r = LLGL::CastTo<LLGL::RenderTarget>(&resource)) {
        m_context->Release(*r);
    }
    else if (LLGL::CommandBuffer* r = LLGL::CastTo<LLGL::CommandBuffer>(&resource)) {
        m_context->Release(*r);
    }
    else {
        SGE_UNREACHABLE();
    }
}

#if SGE_DEBUG
LLGL::FrameProfile sge::RenderContext::GetDebugInfo() {
    LLGL::FrameProfile profile;
    m_debugger->FlushProfile(&profile);
    return profile;
}
#endif