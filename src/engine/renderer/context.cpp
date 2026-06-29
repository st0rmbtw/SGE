#include <filesystem>
#include <fstream>

#include <LLGL/Constants.h>
#include <LLGL/Format.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/TextureFlags.h>

#include <SGE/assert.hpp>
#include <SGE/defines.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/types.hpp>

#if SGE_IMGUI_ENABLED
    #include <imgui.h>
    #include <backends/imgui_impl_glfw.h>
    #include "imgui_renderer.hpp"
#endif

namespace fs = std::filesystem;

bool sge::RenderContext::Init(sge::RenderBackend backend, ImGuiConfig imguiConfig) {
    m_backend = backend;
    m_imgui_config = imguiConfig;

    LLGL::Report report;

    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName = backend.ToString();

    if (backend.IsOpenGL()) {
        LLGL::RendererConfigurationOpenGL* config = new LLGL::RendererConfigurationOpenGL();
        config->contextProfile = LLGL::OpenGLContextProfile::CoreProfile;

        rendererDesc.rendererConfig = config;
        rendererDesc.rendererConfigSize = sizeof(LLGL::RendererConfigurationOpenGL);
    }

#if SGE_DEBUG_LAYER_ENABLED
    m_debugger = new LLGL::RenderingDebugger();
    #if SGE_DEBUG
    rendererDesc.flags    = LLGL::RenderSystemFlags::DebugDevice;
    #endif
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

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;
    m_command_buffer = m_context->CreateCommandBuffer(command_buffer_desc);

    LLGL::SamplerDescriptor desc;
    desc.minFilter = LLGL::SamplerFilter::Linear;
    desc.magFilter = LLGL::SamplerFilter::Linear;
    desc.mipMapFilter = LLGL::SamplerFilter::Linear;
    desc.addressModeU = LLGL::SamplerAddressMode::Clamp;
    desc.addressModeV = LLGL::SamplerAddressMode::Clamp;
    desc.addressModeW = LLGL::SamplerAddressMode::Clamp;
    m_linear_sampler = CreateSampler(desc);

    desc.minFilter = LLGL::SamplerFilter::Nearest;
    desc.magFilter = LLGL::SamplerFilter::Nearest;
    desc.mipMapFilter = LLGL::SamplerFilter::Nearest;
    m_nearest_sampler = CreateSampler(desc);

    const LLGL::RendererInfo& info = GetRendererInfo();

    SGE_LOG_INFO("{:<20} {}", "Renderer:", info.rendererName.c_str());
    SGE_LOG_INFO("{:<20} {}", "Device:", info.deviceName.c_str());
    SGE_LOG_INFO("{:<20} {}", "Vendor:", info.vendorName.c_str());
    SGE_LOG_INFO("{:<20} {}", "Shading Language:", info.shadingLanguageName.c_str());
    if (m_debugger != nullptr) {
        SGE_LOG_INFO("{:<20} Enabled", "Debug Layer:");
    } else {
        SGE_LOG_INFO("{:<20} Disabled", "Debug Layer:");
    }

    SGE_LOG_INFO("Extensions:");
    for (const auto& extension : info.extensionNames) {
        SGE_LOG_INFO("  {}", extension.c_str());
    }

    return true;
}

void sge::RenderContext::Destroy() {
    m_context->GetCommandQueue()->WaitIdle();

#if SGE_IMGUI_ENABLED
    for (auto& [key, imguiContext] : m_imgui_context_map) {
        ImGui::SetCurrentContext(imguiContext);
        ImGuiRenderer::Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imguiContext);
    }
#endif

    for (auto& [key, swapChain] : m_swapchain_map) {
        Release(*swapChain);
    }

    for (auto& [key, renderPass] : m_render_passes) {
        Release(*renderPass);
    }

    for (auto& [key, renderTarget] : m_render_targets) {
        Release(*renderTarget.handle);
    }

    for (auto& [key, pipelineState] : m_pipeline_states) {
        Release(*pipelineState);
    }

    m_pipeline_configs.clear();
    m_render_target_configs.clear();
    m_render_pass_configs.clear();
}

sge::RenderContext::~RenderContext() {
    if (m_context != nullptr) {
        m_context->GetCommandQueue()->WaitIdle();
    }

#if SGE_DEBUG_LAYER_ENABLED
    if (m_debugger != nullptr) {
        delete m_debugger;
    }
#endif

    if (m_context != nullptr) {
        m_context->Release(*m_command_buffer);
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

#if SGE_IMGUI_ENABLED
    ReleaseImGuiContext(window);
#endif
}

LLGL::SwapChain& sge::RenderContext::GetOrCreateSwapChain(const std::shared_ptr<GlfwWindow>& window) {
    ZoneScoped;

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

LLGL::SwapChain* sge::RenderContext::GetSwapChain(const GlfwWindow& window) {
    auto it = m_swapchain_map.find(window.GetID());
    if (it == m_swapchain_map.end()) {
        return nullptr;
    }
    return it->second;
}

void sge::RenderContext::Present(const sge::GlfwWindow& window) {
    ZoneScoped;

    auto it = m_swapchain_map.find(window.GetID());
    SGE_ASSERT(it != m_swapchain_map.end());

    it->second->Present();
}

LLGL::PipelineState& sge::RenderContext::GetOrCreatePipeline(Handle<LLGL::PipelineState> handle) {
    ZoneScoped;

    SGE_ASSERT(GetCurrentTarget() != nullptr);

    const auto it_config = m_pipeline_configs.find(handle.ID());
    SGE_ASSERT(it_config != m_pipeline_configs.end());

    const GraphicsPipelineConfig& config = it_config->second;

    LLGL::PipelineState* pipeline_state = nullptr;

    const PipelineConfigKey key = {.render_target=GetCurrentTarget(), .config_id=handle.ID()};
    const auto it_pipeline = m_pipeline_states.find(key);
    if (it_pipeline != m_pipeline_states.end()) {
        pipeline_state = it_pipeline->second;
    } else {
        LLGL::RenderPass* renderPass = nullptr;
        if (config.renderPass.IsValid()) {
            renderPass = &GetOrCreateRenderPass(config.renderPass, GetCurrentTarget()->GetSamples());
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
        pipelineDesc.rasterizer.multiSampleEnabled = (GetCurrentTarget()->GetSamples() > 1);
        
        if (pipelineDesc.renderPass == nullptr) {
            pipelineDesc.renderPass = GetCurrentTarget()->GetRenderPass();
        }

        pipeline_state = CreatePipelineState(pipelineDesc);
        SGE_ASSERT(pipeline_state != nullptr);

        m_pipeline_states[key] = pipeline_state;
    }

    return *pipeline_state;
}

LLGL::RenderTarget& sge::RenderContext::GetOrCreateRenderTarget(Handle<LLGL::RenderTarget> handle, uint8_t samples) {
    ZoneScoped;

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
        renderTarget = it_target->second.handle;
    } else {
        CachedRenderTarget cachedRenderTarget;

        LLGL::RenderPass* renderPass = nullptr;
        if (config.renderPass.IsValid()) {
            renderPass = &GetOrCreateRenderPass(config.renderPass, samples);
        }

        LLGL::RenderTargetDescriptor targetDesc;
        targetDesc.resolution = config.resolution;
        targetDesc.samples = samples;
        targetDesc.renderPass = renderPass;

        if (samples > 1) {
            for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
                targetDesc.resolveAttachments[i].format = config.colorAttachments[i].format;
                targetDesc.resolveAttachments[i].texture = config.colorAttachments[i].texture;
                targetDesc.resolveAttachments[i].mipLevel = config.colorAttachments[i].mipLevel;
                targetDesc.resolveAttachments[i].arrayLayer = config.colorAttachments[i].arrayLayer;
            }

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
                    LLGL::Texture* texture = CreateTexture(textureDesc);
                    targetDesc.colorAttachments[i].texture = texture;
                    cachedRenderTarget.colorAttachmentTextures[i] = texture;
                }
            }
        } else {
            for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
                targetDesc.colorAttachments[i].format = config.colorAttachments[i].format;
                targetDesc.colorAttachments[i].texture = config.colorAttachments[i].texture;
                targetDesc.colorAttachments[i].mipLevel = config.colorAttachments[i].mipLevel;
                targetDesc.colorAttachments[i].arrayLayer = config.colorAttachments[i].arrayLayer;
            }
        }

        targetDesc.depthStencilAttachment.format = config.depthStencilAttachment.format;
        targetDesc.depthStencilAttachment.texture = config.depthStencilAttachment.texture;
        targetDesc.depthStencilAttachment.mipLevel = config.depthStencilAttachment.mipLevel;
        targetDesc.depthStencilAttachment.arrayLayer = config.depthStencilAttachment.arrayLayer;

        renderTarget = CreateRenderTarget(targetDesc);
        cachedRenderTarget.handle = renderTarget;

        m_render_targets.try_emplace(key, cachedRenderTarget);
    }

    return *renderTarget;
}

LLGL::RenderPass& sge::RenderContext::GetOrCreateRenderPass(Handle<LLGL::RenderPass> handle, uint8_t samples) {
    ZoneScoped;

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

        renderPass = CreateRenderPass(targetDesc);

        m_render_passes.try_emplace(key, renderPass);
    }

    return *renderPass;
}

#if SGE_IMGUI_ENABLED

ImGuiContext* sge::RenderContext::GetImGuiContext(GlfwWindow& window) {
    auto it = m_imgui_context_map.find(window.GetID());
    if (it == m_imgui_context_map.end())
        return nullptr;
    return it->second;
}

ImGuiContext* sge::RenderContext::GetOrCreateImGuiContext(GlfwWindow& window) {
    ZoneScoped;

    ImGuiContext* context = GetImGuiContext(window);
    if (!context) {
        context = ImGui::CreateContext();
        {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = m_imgui_config.ConfigFlags;
            io.ConfigNavSwapGamepadButtons = m_imgui_config.ConfigNavSwapGamepadButtons;
            io.ConfigNavMoveSetMousePos = m_imgui_config.ConfigNavMoveSetMousePos;
            io.ConfigNavCaptureKeyboard = m_imgui_config.ConfigNavCaptureKeyboard;
            io.ConfigNavEscapeClearFocusItem = m_imgui_config.ConfigNavEscapeClearFocusItem;
            io.ConfigNavEscapeClearFocusWindow = m_imgui_config.ConfigNavEscapeClearFocusWindow;
            io.ConfigNavCursorVisibleAuto = m_imgui_config.ConfigNavCursorVisibleAuto;
            io.ConfigNavCursorVisibleAlways = m_imgui_config.ConfigNavCursorVisibleAlways;
            io.ConfigDockingNoSplit = m_imgui_config.ConfigDockingNoSplit;
            io.ConfigDockingNoDockingOver = m_imgui_config.ConfigDockingNoDockingOver;
            io.ConfigDockingWithShift = m_imgui_config.ConfigDockingWithShift;
            io.ConfigDockingAlwaysTabBar = m_imgui_config.ConfigDockingAlwaysTabBar;
            io.ConfigDockingTransparentPayload = m_imgui_config.ConfigDockingTransparentPayload;
            io.ConfigViewportsNoAutoMerge = m_imgui_config.ConfigViewportsNoAutoMerge;
            io.ConfigViewportsNoTaskBarIcon = m_imgui_config.ConfigViewportsNoTaskBarIcon;
            io.ConfigViewportsNoDecoration = m_imgui_config.ConfigViewportsNoDecoration;
            io.ConfigViewportsNoDefaultParent = m_imgui_config.ConfigViewportsNoDefaultParent;
            io.ConfigViewportsPlatformFocusSetsImGuiFocus = m_imgui_config.ConfigViewportsPlatformFocusSetsImGuiFocus;
        }
        ImGui::SetCurrentContext(context);
        ImGui::StyleColorsDark();

        LLGL::NativeHandle nativeHandle;
        window.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

        ImGui_ImplGlfw_InitForOther(window.GetGlfwHandle(), false);
        ImGuiRenderer::Init(shared_from_this());
    
        m_imgui_context_map.try_emplace(window.GetID(), context);
    }
    
    return context;
}

void sge::RenderContext::ReleaseImGuiContext(const GlfwWindow& window) {
    auto it = m_imgui_context_map.find(window.GetID());
    if (it == m_imgui_context_map.end()) {
        return;
    }

    ImGui::SetCurrentContext(it->second);

    ImGuiRenderer::Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext(it->second);

    m_imgui_context_map.erase(it);
}

void sge::RenderContext::BeginImGuiFrame(GlfwWindow& window) {
    ZoneScoped;

    ImGui::SetCurrentContext(GetOrCreateImGuiContext(window));
    ImGui_ImplGlfw_NewFrame();
    ImGuiRenderer::NewFrame();
}

void sge::RenderContext::EndImGuiFrame() {
    ZoneScoped;

    ImGuiRenderer::RenderDrawData(ImGui::GetDrawData());
}

#endif

void sge::RenderContext::DeletePipeline(Handle<LLGL::PipelineState> handle) {
    if (!handle.IsValid()) return;

    for (auto it = m_pipeline_states.begin(); it != m_pipeline_states.end();) {
        PipelineConfigKey key = it->first;
        LLGL::PipelineState* value = it->second;

        if (key.config_id == handle.ID()) {
            Release(*value);
            it = m_pipeline_states.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = m_pipeline_configs.begin(); it != m_pipeline_configs.end();) {
        auto key = it->first;
        auto value = it->second;

        if (key == handle.ID()) {
            it = m_pipeline_configs.erase(it);
            continue;
        }

        ++it;
    }
}

void sge::RenderContext::DeleteRenderTarget(Handle<LLGL::RenderTarget> handle) {
    if (!handle.IsValid()) return;

    for (auto it = m_render_targets.begin(); it != m_render_targets.end();) {
        auto key = it->first;
        auto value = it->second;

        if (key.config_id == handle.ID()) {
            Release(*value.handle);
            for (LLGL::Texture* texture : value.colorAttachmentTextures) {
                if (texture != nullptr) {
                    Release(*texture);
                }
            }

            it = m_render_targets.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = m_render_target_configs.begin(); it != m_render_target_configs.end();) {
        auto key = it->first;

        if (key == handle.ID()) {
            it = m_render_target_configs.erase(it);
            continue;
        }

        ++it;
    }
}

sge::Raw<sge::Sampler> sge::RenderContext::CreateSampler(const LLGL::SamplerDescriptor& descriptor) {
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    SGE_ASSERT(sampler != nullptr);
    return Raw<sge::Sampler>::Create(new Sampler(CreateUnique(sampler), descriptor));
}

sge::Texture sge::RenderContext::CreateTexture(const sge::TextureConfig& config, const LLGL::ImageView* initialData) {
    ZoneScoped;

    LLGL::TextureDescriptor texture_desc;
    texture_desc.debugName = config.debugName;
    texture_desc.type = config.textureType;
    texture_desc.extent = config.extent;
    texture_desc.arrayLayers = config.arrayLayers;
    texture_desc.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment;
    texture_desc.cpuAccessFlags = 0;
    texture_desc.mipLevels = config.generateMipMaps ? 0 : 1;
    texture_desc.miscFlags = LLGL::MiscFlags::FixedSamples;
    texture_desc.miscFlags |= LLGL::MiscFlags::GenerateMips * config.generateMipMaps;

    if (initialData == nullptr) {
        texture_desc.miscFlags |= LLGL::MiscFlags::NoInitialData;
    }

    uint32_t id = m_texture_index++;

    LLGL::Texture* texture = CreateTexture(texture_desc, initialData);
    SGE_ASSERT(texture != nullptr);

    return sge::Texture(id, sge::Size(config.extent.width, config.extent.height), config.sampler, Ref<LLGL::Texture>(shared_from_this(), texture));
}

sge::Raw<LLGL::PipelineState> sge::RenderContext::CreatePipelineState(const LLGL::GraphicsPipelineDescriptor& desc) {
    LLGL::PipelineState* pipelineState = m_context->CreatePipelineState(desc);
    if (const LLGL::Report* report = pipelineState->GetReport()) {
        if (*report->GetText() != '\0') {
            if (report->HasErrors()) {
                SGE_DEBUG_BREAK();
                SGE_LOG_ERROR("Failed to create a pipeline. Error: {}", report->GetText());
                return Raw<LLGL::PipelineState>::Create(shared_from_this(), nullptr);
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }
    return Raw<LLGL::PipelineState>::Create(shared_from_this(), pipelineState);
}

sge::Framebuffer sge::RenderContext::CreateFramebuffer(const sge::FramebufferConfig& config) {
    LLGL::RenderTargetDescriptor targetDesc;
    targetDesc.debugName = config.debugName;
    targetDesc.resolution = config.resolution;
    targetDesc.renderPass = config.renderPass;
    targetDesc.samples = config.samples;

    std::array<sge::Ref<LLGL::Texture>, LLGL_MAX_NUM_COLOR_ATTACHMENTS> textures;
    std::array<sge::Ref<LLGL::Texture>, LLGL_MAX_NUM_COLOR_ATTACHMENTS> msTextures;

    LLGL::TextureDescriptor textureDesc;
    textureDesc.type = LLGL::TextureType::Texture2D;
    textureDesc.extent.width = config.resolution.width;
    textureDesc.extent.height = config.resolution.height;
    textureDesc.mipLevels = 1;

    if (config.samples > 1) {
        LLGL::TextureDescriptor resolveTextureDesc = textureDesc;

        textureDesc.type = LLGL::TextureType::Texture2DMS;
        textureDesc.samples = config.samples;

        for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
            const sge::AttachmentConfig& attachment = config.colorAttachments[i];

            sge::Ref<LLGL::Texture> resolveTexture = attachment.texture;
            if (!resolveTexture && attachment.format != LLGL::Format::Undefined) {
                resolveTextureDesc.format = attachment.format;
                resolveTextureDesc.bindFlags = attachment.bindFlags;
                resolveTexture = CreateTexture(resolveTextureDesc);
            }

            if (resolveTexture) {
                textureDesc.format = attachment.format;
                sge::Ref<LLGL::Texture> msTexture = CreateTexture(textureDesc);
                
                targetDesc.colorAttachments[i].format = attachment.format;
                targetDesc.colorAttachments[i].texture = msTexture;
                msTextures[i] = msTexture;
            }
            
            targetDesc.resolveAttachments[i].format = attachment.format;
            targetDesc.resolveAttachments[i].texture = resolveTexture;
            targetDesc.resolveAttachments[i].mipLevel = attachment.mipLevel;
            targetDesc.resolveAttachments[i].arrayLayer = attachment.arrayLayer;
            textures[i] = resolveTexture;
        }
    } else {
        for (uint8_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i) {
            const sge::AttachmentConfig& attachment = config.colorAttachments[i];

            sge::Ref<LLGL::Texture> texture = attachment.texture;
            if (!texture && attachment.format != LLGL::Format::Undefined) {
                textureDesc.format = attachment.format;
                textureDesc.bindFlags = attachment.bindFlags;
                texture = CreateTexture(textureDesc);
            }

            targetDesc.colorAttachments[i].format = attachment.format;
            targetDesc.colorAttachments[i].texture = texture;
            targetDesc.colorAttachments[i].mipLevel = attachment.mipLevel;
            targetDesc.colorAttachments[i].arrayLayer = attachment.arrayLayer;

            textures[i] = texture;
        }
    }

    sge::Ref<LLGL::Texture> depthStencilTexture;
    if (config.depthStencilAttachment.format != LLGL::Format::Undefined) {
        textureDesc.format = config.depthStencilAttachment.format;
        textureDesc.bindFlags = config.depthStencilAttachment.bindFlags;
        textureDesc.samples = config.samples;   
        depthStencilTexture = CreateTexture(textureDesc);
    }
    
    targetDesc.depthStencilAttachment.texture = depthStencilTexture;

    return sge::Framebuffer(CreateRenderTarget(targetDesc), textures, msTextures, depthStencilTexture, config.renderPass);
}

sge::TemporaryFramebuffer sge::RenderContext::GetTemporaryFramebuffer(LLGL::Extent2D resolution, LLGL::Format format, uint8_t samples, long bindFlags) {
    auto key = TemporaryFramebufferKey {
        .width = resolution.width,
        .height = resolution.height,
        .bindFlags = bindFlags,
        .format = format,
        .samples = samples
    };

    auto fb = m_temp_framebuffer_pool.Get(key);

    if (!fb.IsValid()) {
        sge::FramebufferConfig framebufferConfig;
        framebufferConfig.colorAttachments[0].format = format;
        framebufferConfig.colorAttachments[0].bindFlags = bindFlags;
        framebufferConfig.depthStencilAttachment.format = LLGL::Format::D24UNormS8UInt;
        framebufferConfig.depthStencilAttachment.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::DepthStencilAttachment;
        framebufferConfig.resolution = resolution;
        framebufferConfig.samples = samples;
        fb = m_temp_framebuffer_pool.Add(key, CreateFramebuffer(framebufferConfig));
        SGE_ASSERT(fb.IsValid());
    }

    return fb;
}

sge::Raw<LLGL::Shader> sge::RenderContext::LoadShaderFromFile(const ShaderPath& shader_path, const std::vector<ShaderDef>& shader_defs, const sge::ShaderConfig& config) {
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

    LLGL::Shader* shader = CreateShader(shader_path.shader_type, entry_point, data, data_length, config);
    delete[] data;

    if (!shader) {
        std::println(stderr, "Shader file: {}", path_str);
    }
    return sge::Raw<LLGL::Shader>::Create(shared_from_this(), shader);
}

sge::Raw<LLGL::Shader> sge::RenderContext::CreateShader(sge::ShaderType shader_type, const char* entry_point, const void* data, size_t length, const sge::ShaderConfig& config) {
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

    shader_desc.vertex = config.vertex;
    shader_desc.fragment = config.fragment;

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
                SGE_DEBUG_BREAK();
                SGE_LOG_ERROR("Failed to create a shader. Error: {}", report->GetText());
                return sge::Raw<LLGL::Shader>::Create(shared_from_this(), nullptr);
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    return sge::Raw<LLGL::Shader>::Create(shared_from_this(), shader);
}

void sge::RenderContext::ReleaseUntyped(LLGL::RenderSystemChild& resource) {
    if (auto* r = LLGL::CastTo<LLGL::Buffer>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::BufferArray>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::Texture>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::Sampler>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::Shader>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::PipelineLayout>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::PipelineState>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::RenderPass>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::ResourceHeap>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::PipelineCache>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::QueryHeap>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::Fence>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::SwapChain>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::RenderTarget>(&resource)) {
        m_context->Release(*r);
    }
    else if (auto* r = LLGL::CastTo<LLGL::CommandBuffer>(&resource)) {
        m_context->Release(*r);
    }
    else {
        SGE_UNREACHABLE();
    }
}

#if SGE_DEBUG_LAYER_ENABLED
void sge::RenderContext::GetFrameProfile(LLGL::FrameProfile* profile) {
    m_debugger->FlushProfile(profile);
}
#endif
