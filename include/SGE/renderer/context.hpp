#pragma once

#ifndef SGE_ENGINE_RENDERER_CONTEXT_HPP
#define SGE_ENGINE_RENDERER_CONTEXT_HPP

#include <SGE/renderer/framebuffer_pool.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/framebuffer.hpp>
#include <SGE/types/sampler.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/utils/hash.hpp>

#include <LLGL/RenderSystemChild.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Utils/VertexFormat.h>

#include <imgui.h>

#include <concepts>
#include <filesystem>
#include <stack>

struct PipelineConfigKey {
    LLGL::RenderTarget* render_target;
    uint32_t config_id;
};

inline bool operator==(const PipelineConfigKey& a, const PipelineConfigKey& b) {
    return a.config_id == b.config_id && a.render_target == b.render_target;
}

template <>
struct std::hash<PipelineConfigKey> {
    static size_t operator()(const PipelineConfigKey& key) noexcept {
        size_t seed = 0;
        hash_combine(seed, key.config_id);
        hash_combine(seed, key.render_target);
        return seed;
    }
};

struct RenderTargetKey {
    LLGL::Extent2D resolution;
    uint32_t config_id = -1;
    uint32_t render_pass_id = -1;
    LLGL::Format format = LLGL::Format::Undefined;
    uint8_t samples = 1;
};

inline bool operator==(const RenderTargetKey& a, const RenderTargetKey& b) {
    return a.config_id == b.config_id
        && a.samples == b.samples
        && a.format == b.format
        && a.resolution == b.resolution
        && a.render_pass_id == b.render_pass_id;
}

template <>
struct std::hash<RenderTargetKey> {
    static size_t operator()(const RenderTargetKey& key) noexcept {
        size_t seed = 0;
        hash_combine(seed, key.config_id);
        hash_combine(seed, key.render_pass_id);
        hash_combine(seed, key.samples);
        hash_combine(seed, key.format);
        hash_combine(seed, key.resolution.width);
        hash_combine(seed, key.resolution.height);
        return seed;
    }
};

struct RenderPassKey {
    uint32_t config_id = -1;
    uint8_t samples = 1;
};

inline bool operator==(const RenderPassKey& a, const RenderPassKey& b) {
    return a.config_id == b.config_id && a.samples == b.samples;
}

template <>
struct std::hash<RenderPassKey> {
    static size_t operator()(const RenderPassKey& key) noexcept {
        size_t seed = 0;
        hash_combine(seed, key.config_id);
        hash_combine(seed, key.samples);
        return seed;
    }
};


namespace sge {

struct ImGuiConfig {
    int ConfigFlags = ImGuiConfigFlags_None;

    bool ConfigNavSwapGamepadButtons = false;
    bool ConfigNavMoveSetMousePos = false;
    bool ConfigNavCaptureKeyboard = true;
    bool ConfigNavEscapeClearFocusItem = true;
    bool ConfigNavEscapeClearFocusWindow = false;
    bool ConfigNavCursorVisibleAuto = true;
    bool ConfigNavCursorVisibleAlways = false;

    bool ConfigDockingNoSplit = false;
    bool ConfigDockingNoDockingOver = false;
    bool ConfigDockingWithShift = false;
    bool ConfigDockingAlwaysTabBar = false;
    bool ConfigDockingTransparentPayload = false;

    bool ConfigViewportsNoAutoMerge = false;
    bool ConfigViewportsNoTaskBarIcon = false;
    bool ConfigViewportsNoDecoration = true;
    bool ConfigViewportsNoDefaultParent = true;
    bool ConfigViewportsPlatformFocusSetsImGuiFocus = true;
};

class RenderContext : public std::enable_shared_from_this<RenderContext> {
public:
    RenderContext() = default;

    bool Init(RenderBackend backend, ImGuiConfig imguiConfig);
    void Destroy();
    ~RenderContext();
    
    void UnregisterWindow(const GlfwWindow& window);

    LLGL::PipelineState& GetOrCreatePipeline(Handle<LLGL::PipelineState> handle);
    LLGL::RenderTarget& GetOrCreateRenderTarget(Handle<LLGL::RenderTarget> handle, uint8_t samples);
    LLGL::RenderPass& GetOrCreateRenderPass(Handle<LLGL::RenderPass> handle, uint8_t samples);
    LLGL::SwapChain& GetOrCreateSwapChain(const std::shared_ptr<GlfwWindow>& window);
    LLGL::SwapChain* GetSwapChain(const GlfwWindow& window);

#if SGE_IMGUI_ENABLED
    ImGuiContext* GetOrCreateImGuiContext(GlfwWindow& window);
    ImGuiContext* GetImGuiContext(GlfwWindow& window);
    void ReleaseImGuiContext(const GlfwWindow& window);
#endif

#if SGE_IMGUI_ENABLED
    void BeginImGuiFrame(GlfwWindow& window);
    void EndImGuiFrame();
#else
    inline void BeginImGuiFrame(GlfwWindow&) {}
    inline void EndImGuiFrame() {}
#endif

    void DeletePipeline(Handle<LLGL::PipelineState> handle);
    void DeleteRenderTarget(Handle<LLGL::RenderTarget> renderTarget);

    void Present(const GlfwWindow& window);

    void ReleaseUntyped(LLGL::RenderSystemChild& resource);

    template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
    inline void Release(T& resource) {
        m_context->Release(resource);
    }

    inline void Release(Handle<LLGL::RenderTarget> renderTarget) {
        DeleteRenderTarget(renderTarget);
    }

    sge::Ref<Sampler>& GetLinearSampler() {
        return m_linear_sampler;
    }
    
    sge::Ref<Sampler>& GetNearestSampler() {
        return m_nearest_sampler;
    }
    
    Raw<Sampler> CreateSampler(const LLGL::SamplerDescriptor& descriptor);

    Texture CreateTexture(const sge::TextureConfig& config, const LLGL::ImageView* initialData = nullptr);

    Framebuffer CreateFramebuffer(const sge::FramebufferConfig& config);

    Raw<LLGL::Texture> CreateTexture(const LLGL::TextureDescriptor& desc, const LLGL::ImageView* initialImage = nullptr) {
        return Raw<LLGL::Texture>::Create(shared_from_this(), m_context->CreateTexture(desc, initialImage));
    }

    inline sge::Texture CopyTextureWithSampler(const sge::Texture& texture, const Ref<sge::Sampler>& sampler) {
        return Texture(m_texture_index++, texture.size(), sampler, texture.internal());
    }

    Raw<LLGL::Shader> LoadShaderFromFile(const sge::ShaderPath& shader_path, const std::vector<sge::ShaderDef>& shader_defs = {}, const sge::ShaderConfig& config = {});
    Raw<LLGL::Shader> CreateShader(sge::ShaderType shader_type, const char* entry_point, const void* data, size_t length, const sge::ShaderConfig& config = {});

    LLGL::PipelineCache* ReadPipelineCache(const std::filesystem::path& dir, const std::string& name, bool& hasInitialCache);
    void SavePipelineCache(const std::filesystem::path& dir, const std::string& name, LLGL::PipelineCache& pipelineCache);

    inline Raw<LLGL::RenderTarget> CreateRenderTarget(const LLGL::RenderTargetDescriptor& desc) {
        return Raw<LLGL::RenderTarget>::Create(shared_from_this(), m_context->CreateRenderTarget(desc));
    }

    Raw<LLGL::RenderTarget> CreateRenderTargetFromTexture(LLGL::Texture& texture) {
        const LLGL::TextureDescriptor& textureDesc = texture.GetDesc();

        LLGL::RenderTargetDescriptor targetDesc;
        targetDesc.resolution.width = textureDesc.extent.width;
        targetDesc.resolution.height = textureDesc.extent.height;
        targetDesc.samples = textureDesc.samples;
        targetDesc.colorAttachments[0] = &texture;
        return CreateRenderTarget(targetDesc);
    }

    inline Raw<LLGL::Buffer> CreateBuffer(const LLGL::BufferDescriptor& desc, const void* initial_data = nullptr) {
        return Raw<LLGL::Buffer>::Create(shared_from_this(), m_context->CreateBuffer(desc, initial_data));
    }

    inline Raw<LLGL::BufferArray> CreateBufferArray(std::initializer_list<LLGL::Buffer*> buffers) {
        return Raw<LLGL::BufferArray>::Create(shared_from_this(), m_context->CreateBufferArray(buffers.size(), buffers.begin()));
    }

    template <typename Container>
    inline Raw<LLGL::Buffer> CreateVertexBuffer(const Container& vertices, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size           = GetArraySize(vertices);
        bufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
        bufferDesc.vertexAttribs  = vertexFormat.attributes;
        bufferDesc.debugName      = debug_name;
        return CreateBuffer(bufferDesc, &vertices[0]);
    }

    inline Raw<LLGL::Buffer> CreateVertexBuffer(size_t size, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size           = size;
        bufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
        bufferDesc.vertexAttribs  = vertexFormat.attributes;
        bufferDesc.debugName      = debug_name;
        return CreateBuffer(bufferDesc);
    }

    template <typename Container>
    inline Raw<LLGL::Buffer> CreateIndexBuffer(const Container& indices, const LLGL::Format format, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size           = GetArraySize(indices);
        bufferDesc.bindFlags      = LLGL::BindFlags::IndexBuffer;
        bufferDesc.format         = format;
        bufferDesc.debugName      = debug_name;
        return CreateBuffer(bufferDesc, &indices[0]);
    }

    inline Raw<LLGL::Buffer> CreateConstantBuffer(const size_t size, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc;
        bufferDesc.size           = size;
        bufferDesc.bindFlags      = LLGL::BindFlags::ConstantBuffer;
        bufferDesc.miscFlags      = LLGL::MiscFlags::DynamicUsage;
        bufferDesc.debugName      = debug_name;
        return CreateBuffer(bufferDesc);
    }

    inline Raw<LLGL::CommandBuffer> CreateCommandBuffer(const LLGL::CommandBufferDescriptor& desc = {}) {
        return Raw<LLGL::CommandBuffer>::Create(shared_from_this(), m_context->CreateCommandBuffer(desc));
    }

    inline Raw<LLGL::PipelineLayout> CreatePipelineLayout(const LLGL::PipelineLayoutDescriptor& desc) {
        return Raw<LLGL::PipelineLayout>::Create(shared_from_this(), m_context->CreatePipelineLayout(desc));
    }

    inline Raw<LLGL::PipelineState> CreatePipelineState(const LLGL::GraphicsPipelineDescriptor& desc) {
        return Raw<LLGL::PipelineState>::Create(shared_from_this(), m_context->CreatePipelineState(desc));
    }

    inline Raw<LLGL::RenderPass> CreateRenderPass(const LLGL::RenderPassDescriptor& desc) {
        return Raw<LLGL::RenderPass>::Create(shared_from_this(), m_context->CreateRenderPass(desc));
    }

    inline Raw<LLGL::ResourceHeap> CreateResourceHeap(LLGL::PipelineLayout* pipelineLayout, LLGL::ArrayView<LLGL::ResourceViewDescriptor> resource_views) {
        LLGL::ResourceHeapDescriptor desc;
        desc.pipelineLayout = pipelineLayout;
        desc.numResourceViews = resource_views.size();
        return Raw<LLGL::ResourceHeap>::Create(shared_from_this(), m_context->CreateResourceHeap(desc, resource_views));
    }

    inline Raw<LLGL::PipelineState> CreateComputePipelineState(const ComputePipelineConfig& config) {
        LLGL::ComputePipelineDescriptor desc;
        desc.debugName = config.debugName;
        desc.pipelineLayout = config.pipelineLayout;
        desc.computeShader = config.computeShader;
        return Raw<LLGL::PipelineState>::Create(shared_from_this(), m_context->CreatePipelineState(desc));
    }

    inline Handle<LLGL::PipelineState> CreatePipelineState(const GraphicsPipelineConfig& config) {
        uint32_t index = m_pipeline_config_index;
        m_pipeline_configs[m_pipeline_config_index++] = config;
        return Handle<LLGL::PipelineState>(index);
    }

    inline Handle<LLGL::RenderPass> CreateRenderPass(const RenderPassConfig& config) {
        uint32_t index = m_render_pass_config_index;
        m_render_pass_configs[m_render_pass_config_index++] = config;
        return Handle<LLGL::RenderPass>(index);
    }

    inline Handle<LLGL::RenderTarget> CreateRenderTarget(const RenderTargetConfig& config) {
        const uint32_t id = m_render_target_config_index;
        m_render_target_configs.try_emplace(m_render_target_config_index++, config);
        return Handle<LLGL::RenderTarget>(id);
    }

    void PushRenderTarget(LLGL::RenderTarget* target) {
        m_target_stack.push(target);
    }

    LLGL::RenderTarget* PopRenderTarget() {
        LLGL::RenderTarget* target = m_target_stack.top();
        m_target_stack.pop();
        return target;
    }

    template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
    Ref<T> CreateRef(T* resource) {
        return Ref<T>(shared_from_this(), resource);
    }

    template <typename T> requires std::derived_from<T, LLGL::RenderSystemChild>
    Unique<T> CreateUnique(T* resource) {
        return Unique<T>(shared_from_this(), resource);
    }

    sge::TemporaryFramebuffer GetTemporaryFramebuffer(LLGL::Extent2D resolution, LLGL::Format format, uint8_t samples = 1, long bindFlags = (LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled));

    inline void TickTemporaryFramebufferPool(uint32_t max_unused_frames = 60) {
        m_temp_framebuffer_pool.Tick(max_unused_frames);
    }

#if SGE_DEBUG
    void GetDebugInfo(LLGL::FrameProfile* profile);
#endif

    [[nodiscard]]
    bool IsInitialized() const noexcept {
        return m_context ? true : false;
    }

    [[nodiscard]]
    inline const LLGL::RenderSystemPtr& GetLLGLContext() const noexcept {
        return m_context;
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* GetCommandQueue() const noexcept {
        return m_context->GetCommandQueue();
    }

    [[nodiscard]]
    inline LLGL::CommandBuffer* GetCommandBuffer() const noexcept {
        return m_command_buffer;
    }

    [[nodiscard]]
    inline const LLGL::RendererInfo& GetRendererInfo() const noexcept {
        return m_context->GetRendererInfo();
    }

    [[nodiscard]]
    inline const LLGL::RenderingCapabilities& GetRenderingCaps() const noexcept {
        return m_context->GetRenderingCaps();
    }

    [[nodiscard]]
    inline sge::RenderBackend Backend() const noexcept {
        return m_backend;
    }

    [[nodiscard]]
    inline LLGL::RenderTarget* GetCurrentTarget() const noexcept {
        return m_target_stack.top();
    }

#if SGE_DEBUG
    [[nodiscard]]
    inline LLGL::RenderingDebugger* Debugger() const noexcept {
        return m_debugger;
    }
#endif

private:
    struct CachedRenderTarget {
        LLGL::RenderTarget* handle = nullptr;
        LLGL::Texture* colorAttachmentTextures[LLGL_MAX_NUM_COLOR_ATTACHMENTS] = {};
    };

private:
    std::unordered_map<uint32_t, LLGL::SwapChain*> m_swapchain_map;
    std::unordered_map<uint32_t, GraphicsPipelineConfig> m_pipeline_configs;
    std::unordered_map<uint32_t, RenderTargetConfig> m_render_target_configs;
    std::unordered_map<uint32_t, RenderPassConfig> m_render_pass_configs;
    std::unordered_map<PipelineConfigKey, LLGL::PipelineState*> m_pipeline_states;
    std::unordered_map<RenderTargetKey, CachedRenderTarget> m_render_targets;
    std::unordered_map<RenderPassKey, LLGL::RenderPass*> m_render_passes;

    TemporaryFramebufferPool m_temp_framebuffer_pool;

#if SGE_IMGUI_ENABLED
    std::unordered_map<uint32_t, ImGuiContext*> m_imgui_context_map;
#endif

    sge::Ref<Sampler> m_linear_sampler;
    sge::Ref<Sampler> m_nearest_sampler;

    LLGL::RenderSystemPtr m_context = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;
    
#if SGE_DEBUG
    LLGL::RenderingDebugger* m_debugger = nullptr;
#endif

    std::stack<LLGL::RenderTarget*, std::vector<LLGL::RenderTarget*>> m_target_stack;

    uint32_t m_pipeline_config_index = 0;
    uint32_t m_render_target_config_index = 0;
    uint32_t m_render_pass_config_index = 0;
    uint32_t m_texture_index = 0;

    sge::RenderBackend m_backend;
    sge::ImGuiConfig m_imgui_config;
};



} // namespace sge

#endif