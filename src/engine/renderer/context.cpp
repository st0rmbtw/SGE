#include <filesystem>
#include <SGE/renderer/context.hpp>
#include <SGE/profile.hpp>

namespace fs = std::filesystem;

bool sge::RenderContext::Init(sge::RenderBackend backend, bool cache_pipelines, const std::string& cache_dir_path) {
    m_backend = backend;
    m_cache_pipelines = cache_pipelines;
    m_cache_pipeline_dir = cache_dir_path;

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

    if (cache_pipelines && (!fs::is_directory(cache_dir_path) || !fs::exists(cache_dir_path))) {
        fs::create_directories(cache_dir_path);
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

sge::RenderContext::~RenderContext() {
#if SGE_DEBUG
    if (m_debugger != nullptr) {
        delete m_debugger;
    }
#endif

    if (m_context != nullptr) {
        for (auto& [id, swap_chain] : m_swapchain_map) {
            m_context->Release(*swap_chain);
        }

        LLGL::RenderSystem::Unload(std::move(m_context));
    }
}

sge::LLGLResource<LLGL::PipelineCache> sge::RenderContext::ReadPipelineCache(const std::string& name, bool& hasInitialCache) {
    // Try to read PSO cache from file
    const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
    const fs::path cachePath = fs::path(m_cache_pipeline_dir) / cacheFilename;

    LLGL::Blob pipelineCacheBlob = LLGL::Blob::CreateFromFile(cachePath.string());
    if (pipelineCacheBlob) {
        hasInitialCache = true;
    }

    return m_context->CreatePipelineCache(pipelineCacheBlob);
}

void sge::RenderContext::SavePipelineCache(const std::string& name, LLGL::PipelineCache& pipelineCache) {
    if (LLGL::Blob psoCache = pipelineCache.GetBlob())
    {
        const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
        const fs::path cachePath = fs::path(m_cache_pipeline_dir) / cacheFilename;

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
        m_context->Release(*it->second);

    m_swapchain_map.erase(window.GetID());
}

LLGL::SwapChain& sge::RenderContext::GetOrCreateSwapChain(const std::shared_ptr<GlfwWindow>& window) {
    LLGL::SwapChain* swap_chain = nullptr;

    auto it = m_swapchain_map.find(window->GetID());

    if (it == m_swapchain_map.end()) {
        const LLGL::RenderSystemPtr& context = m_context;

        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = window->GetContentSize();
        swapChainDesc.fullscreen = window->IsFullscreen();
        swapChainDesc.samples = window->GetSamples();
        // swapChainDesc.colorBits = settings.transparent ? 32 : 24;

        swap_chain = context->CreateSwapChain(swapChainDesc, window);
        m_swapchain_map.try_emplace(window->GetID(), swap_chain);
    } else {
        swap_chain = it->second.get();
    }

    return *swap_chain;
}

LLGL::SwapChain* sge::RenderContext::GetSwapChain(const std::shared_ptr<GlfwWindow>& window) {
    auto it = m_swapchain_map.find(window->GetID());
    if (it == m_swapchain_map.end()) {
        return nullptr;
    }
    return it->second.get();
}

void sge::RenderContext::Present(const std::shared_ptr<sge::GlfwWindow>& window) {
    auto it = m_swapchain_map.find(window->GetID());
    SGE_ASSERT(it != m_swapchain_map.end());

    it->second->Present();
}

LLGL::PipelineState& sge::RenderContext::GetOrCreatePipeline(uint32_t pipeline_id) {
    SGE_ASSERT(m_current_target != nullptr);

    const auto it_config = m_pipeline_configs.find(pipeline_id);
    SGE_ASSERT(it_config != m_pipeline_configs.end());

    const GraphicsPipelineConfig& config = it_config->second;

    LLGL::PipelineState* pipeline_state = nullptr;

    const PipelineConfigKey key = {.config_id=pipeline_id, .render_target=m_current_target};
    const auto it_pipeline = m_pipeline_states.find(key);
    if (it_pipeline != m_pipeline_states.end()) {
        pipeline_state = it_pipeline->second;
    } else {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        pipelineDesc.depth = config.depth;
        pipelineDesc.blend = config.blend;
        pipelineDesc.debugName = config.debugName;
        pipelineDesc.pipelineLayout = config.layout;
        pipelineDesc.vertexShader = config.vertexShader;
        pipelineDesc.geometryShader = config.geometryShader;
        pipelineDesc.fragmentShader = config.pixelShader;
        pipelineDesc.primitiveTopology = config.primitiveTopology;
        pipelineDesc.indexFormat = config.indexFormat;
        pipelineDesc.rasterizer.multiSampleEnabled = (m_current_target->GetSamples() > 1);
        pipelineDesc.rasterizer.scissorTestEnabled = config.scissorTestEnabled;
        pipelineDesc.rasterizer.frontCCW = config.frontCCW;
        pipelineDesc.renderPass = m_current_target->GetRenderPass();

        pipeline_state = m_context->CreatePipelineState(pipelineDesc);
        m_pipeline_states[key] = pipeline_state;
    }

    return *pipeline_state;
}

void sge::RenderContext::DeletePipeline(uint32_t pipeline_id) {
    for (auto it = m_pipeline_states.begin(); it != m_pipeline_states.end();) {
        auto key = it->first;
        auto value = it->second;

        if (key.config_id == pipeline_id) {
            m_context->Release(*value);
            it = m_pipeline_states.erase(it);
        } else {
            ++it;
        }
    }
}

sge::Sampler sge::RenderContext::CreateSampler(const LLGL::SamplerDescriptor& descriptor) {
    ZoneScoped;
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    return Sampler(sampler, descriptor);
}

sge::Texture sge::RenderContext::CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const Sampler& sampler, const void* data, bool generate_mip_maps) {
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

    return sge::Texture(id, sampler, glm::uvec2(width, height), m_context->CreateTexture(texture_desc, &image_view));
}

LLGL::Shader* sge::RenderContext::LoadShader(const ShaderPath& shader_path, const std::vector<ShaderDef>& shader_defs, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
    ZoneScoped;

    const RenderBackend backend = m_backend;
    const ShaderType shader_type = shader_path.shader_type;

    const std::string filename = backend.IsMetal()
        ? std::format("{}{}", shader_path.name, shader_type.FileExtension(backend))
        : shader_type.IsCompute()
            ? std::format("{}.{}.{}{}", shader_path.func_name, shader_path.name, shader_type.Stage(), shader_type.FileExtension(backend))
            : std::format("{}.{}{}", shader_path.name, shader_type.Stage(), shader_type.FileExtension(backend));

    const fs::path path = fs::path(backend.AssetFolder()) / filename;
    const std::string path_str = path.string().c_str();

    if (!fs::exists(path)) {
        SGE_LOG_ERROR("Failed to find shader '{}'", path_str.c_str());
        return nullptr;
    }

    std::string shader_source;

    if (!backend.IsVulkan()) {
        std::ifstream shader_file;
        shader_file.open(path);

        std::stringstream shader_source_str;
        shader_source_str << shader_file.rdbuf();

        shader_source = shader_source_str.str();

        for (const ShaderDef& shader_def : shader_defs) {
            size_t pos;
            while ((pos = shader_source.find(shader_def.name)) != std::string::npos) {
                shader_source.replace(pos, shader_def.name.length(), shader_def.value);
            }
        }

        shader_file.close();
    }

    LLGL::ShaderDescriptor shader_desc;
    shader_desc.type = shader_type.ToLLGLType();
    shader_desc.sourceType = LLGL::ShaderSourceType::CodeString;

    if (shader_type.IsVertex()) {
        shader_desc.vertex.inputAttribs = vertex_attributes;
    }

    if (backend.IsOpenGL() && shader_type.IsFragment()) {
        shader_desc.fragment.outputAttribs = {
            { "frag_color", LLGL::Format::RGBA8UNorm, 0, LLGL::SystemValue::Color }
        };
    }

    if (backend.IsVulkan()) {
        shader_desc.source = path_str.c_str();
        shader_desc.sourceType = LLGL::ShaderSourceType::BinaryFile;
    } else {
        shader_desc.entryPoint = shader_type.IsCompute() ? shader_path.func_name.c_str() : shader_type.EntryPoint(backend);
        shader_desc.source = shader_source.c_str();
        shader_desc.sourceSize = shader_source.length();
        shader_desc.profile = shader_type.Profile(backend);
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
                SGE_LOG_ERROR("Failed to create a shader. File: {}\nError: {}", path_str.c_str(), report->GetText());
                return nullptr;
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    return shader;
}

#if SGE_DEBUG
void sge::RenderContext::PrintDebugInfo() {
    LLGL::FrameProfile profile;
    m_debugger->FlushProfile(&profile);

    SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
}
#endif