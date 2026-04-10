#include "SGE/renderer/dependency_graph.hpp"
#include <filesystem>
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

sge::RenderContext::~RenderContext() {
    m_dependency_graph.DeleteAll();

#if SGE_DEBUG
    if (m_debugger != nullptr) {
        delete m_debugger;
    }
#endif

    if (m_context != nullptr) {
        LLGL::RenderSystem::Unload(std::move(m_context));
    }
}

sge::LLGLResource<LLGL::PipelineCache> sge::RenderContext::ReadPipelineCache(const std::filesystem::path& dir, const std::string& name, bool& hasInitialCache) {
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
        m_dependency_graph.RemoveNode(*it->second);

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
        SGE_ASSERT(swap_chain != nullptr);

        m_dependency_graph.AddNode(*swap_chain);

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
        pipelineDesc.debugName = config.debugName.c_str();
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
        SGE_ASSERT(pipeline_state != nullptr);

        m_dependency_graph.AddNode(*pipeline_state, *config.layout);
        m_dependency_graph.AddNode(*pipeline_state, *config.vertexShader);
        m_dependency_graph.AddNode(*pipeline_state, *config.pixelShader);
        m_dependency_graph.AddNode(*pipeline_state, *config.geometryShader);

        m_pipeline_states[key] = pipeline_state;
    }

    return *pipeline_state;
}

void sge::RenderContext::DeletePipeline(uint32_t pipeline_id) {
    for (auto it = m_pipeline_states.begin(); it != m_pipeline_states.end(); ++it) {
        auto key = it->first;
        auto value = it->second;

        if (key.config_id == pipeline_id) {
            m_dependency_graph.RemoveNode(*value);
            m_pipeline_states.erase(it);
            break;
        }
    }
}

sge::Sampler sge::RenderContext::CreateSampler(const LLGL::SamplerDescriptor& descriptor) {
    ZoneScoped;
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    SGE_ASSERT(sampler != nullptr);
    m_dependency_graph.AddNode(*sampler);
    return Sampler(sampler, descriptor);
}

LLGL::Buffer* sge::RenderContext::CreateBuffer(const LLGL::BufferDescriptor& desc, const void* initial_data) {
    LLGL::Buffer* buffer = m_context->CreateBuffer(desc, initial_data);
    m_dependency_graph.AddNode(*buffer);
    return buffer;
}

LLGL::BufferArray* sge::RenderContext::CreateBufferArray(std::initializer_list<LLGL::Buffer*> buffers) {
    LLGL::BufferArray* buffer_array = m_context->CreateBufferArray(buffers.size(), buffers.begin());
    for (LLGL::Buffer* buffer : buffers) {
        m_dependency_graph.AddNode(*buffer_array, *buffer);
    }
    return buffer_array;
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

    LLGL::Texture* texture = m_context->CreateTexture(texture_desc, &image_view);
    SGE_ASSERT(texture != nullptr);

    m_dependency_graph.AddNode(*texture);

    return sge::Texture(id, sampler, glm::uvec2(width, height), texture);
}

LLGL::Shader* sge::RenderContext::LoadShaderFromFile(const ShaderPath& shader_path, const std::vector<ShaderDef>& shader_defs, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
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

    std::ifstream shader_file;

    uint8_t* data = nullptr;
    uintmax_t data_length = 0;
    
    if (backend.IsVulkan()) {
        shader_file.open(path, std::ios::binary);
        data_length = fs::file_size(path);
        data = new uint8_t[data_length];
        shader_file.read(reinterpret_cast<char*>(data), data_length);
    } else {
        shader_file.open(path);
        std::string shader_source;
        shader_file >> shader_source;

        for (const ShaderDef& shader_def : shader_defs) {
            size_t pos;
            while ((pos = shader_source.find(shader_def.name)) != std::string::npos) {
                shader_source.replace(pos, shader_def.name.length(), shader_def.value);
            }
        }

        data_length = shader_source.size();
        data = new uint8_t[data_length];
        memcpy(data, shader_source.data(), data_length);
    }

    LLGL::Shader* shader = CreateShader(shader_path.shader_type, shader_path.func_name.c_str(), data, data_length, vertex_attributes);
    delete[] data;

    if (shader == nullptr) {
        fmt::println(stderr, "Shader file: {}", path_str);
    }
    return shader;
}

LLGL::Shader* sge::RenderContext::CreateShader(sge::ShaderType shader_type, const char* entry_point, const void* data, size_t length, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
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
                return nullptr;
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    m_dependency_graph.AddNode(*shader);

    return shader;
}

#if SGE_DEBUG
LLGL::FrameProfile sge::RenderContext::GetDebugInfo() {
    LLGL::FrameProfile profile;
    m_debugger->FlushProfile(&profile);
    return profile;
}
#endif