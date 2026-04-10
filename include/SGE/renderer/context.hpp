#pragma once

#ifndef SGE_ENGINE_RENDERER_CONTEXT_HPP
#define SGE_ENGINE_RENDERER_CONTEXT_HPP

#include <SGE/types/backend.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/dependency_graph.hpp>

#include <SGE/types/sampler.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/shader_def.hpp>


#include <LLGL/Utils/Utility.h>

#include <filesystem>

struct PipelineConfigKey {
    uint32_t config_id;
    LLGL::RenderTarget* render_target;
};

inline bool operator==(const PipelineConfigKey& a, const PipelineConfigKey& b) {
    return a.config_id == b.config_id && a.render_target == b.render_target;
}

template <>
struct std::hash<PipelineConfigKey> {
    static size_t operator()(const PipelineConfigKey& key) noexcept {
        size_t hash = std::hash<uint32_t>{}(key.config_id);
        hash ^= std::hash<uint32_t>{}(key.config_id) << 1;
        return hash;
    }
};

namespace sge {

class RenderContext {
public:
    RenderContext() : m_dependency_graph(&m_context) {
    }

    bool Init(RenderBackend backend);
    ~RenderContext();
    
    void UnregisterWindow(const GlfwWindow& window);

    inline uint32_t AddPipelineConfig(const GraphicsPipelineConfig& config) {
        uint32_t index = m_pipeline_config_index;
        m_pipeline_configs[m_pipeline_config_index++] = config;
        return index;
    }

    LLGL::PipelineState& GetOrCreatePipeline(uint32_t pipeline_id);
    LLGL::SwapChain& GetOrCreateSwapChain(const std::shared_ptr<GlfwWindow>& window);
    LLGL::SwapChain* GetSwapChain(const std::shared_ptr<GlfwWindow>& window);

    void DeletePipeline(uint32_t pipeline_id);

    void Present(const std::shared_ptr<GlfwWindow>& window);
    
    Sampler CreateSampler(const LLGL::SamplerDescriptor& descriptor);

    Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const void* data, bool generate_mip_maps = false);

    LLGL::Buffer* CreateBuffer(const LLGL::BufferDescriptor& desc, const void* initial_data = nullptr);

    LLGL::BufferArray* CreateBufferArray(std::initializer_list<LLGL::Buffer*> buffers);

    inline sge::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const uint8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::UInt8, width, height, layers, sampler, data, generate_mip_maps);
    }

    inline sge::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const int8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::Int8, width, height, layers, sampler, data, generate_mip_maps);
    }

    inline sge::Texture CopyTextureWithSampler(const sge::Texture& texture, const sge::Sampler& sampler) {
        return Texture(m_texture_index++, sampler, texture.size(), texture.internal());
    }

    LLGL::Shader* LoadShaderFromFile(const sge::ShaderPath& shader_path, const std::vector<sge::ShaderDef>& shader_defs = {}, const std::vector<LLGL::VertexAttribute>& vertex_attributes = {});
    LLGL::Shader* CreateShader(sge::ShaderType shader_type, const char* entry_point, const void* data, size_t length, const std::vector<LLGL::VertexAttribute>& vertex_attributes = {});

    LLGLResource<LLGL::PipelineCache> ReadPipelineCache(const std::filesystem::path& dir, const std::string& name, bool& hasInitialCache);
    void SavePipelineCache(const std::filesystem::path& dir, const std::string& name, LLGL::PipelineCache& pipelineCache);

    template <typename TResource>
    requires (std::derived_from<TResource, LLGL::RenderSystemChild>)
    inline void DeleteResource(TResource& resource) {
        m_dependency_graph.RemoveNode(resource);
    }

    template <typename Container>
    inline LLGL::Buffer* CreateVertexBuffer(const Container& vertices, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(GetArraySize(vertices), vertexFormat);
        bufferDesc.debugName = debug_name;
        return CreateBuffer(bufferDesc, &vertices[0]);
    }

    inline LLGL::Buffer* CreateVertexBuffer(size_t size, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return CreateBuffer(bufferDesc);
    }

    inline LLGL::Buffer* CreateVertexBufferInit(size_t size, const void* data, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return CreateBuffer(bufferDesc, data);
    }

    template <typename Container>
    inline LLGL::Buffer* CreateIndexBuffer(const Container& indices, const LLGL::Format format, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc = LLGL::IndexBufferDesc(GetArraySize(indices), format);
        bufferDesc.debugName = debug_name;
        return CreateBuffer(bufferDesc, &indices[0]);
    }

    inline LLGL::Buffer* CreateConstantBuffer(const size_t size, const char* debug_name = nullptr) {
        LLGL::BufferDescriptor bufferDesc = LLGL::ConstantBufferDesc(size);
        bufferDesc.debugName = debug_name;
        return CreateBuffer(bufferDesc);
    }

    inline void SetCurrentRenderTarget(LLGL::RenderTarget* target) {
        m_current_target = target;
    }

#if SGE_DEBUG
    LLGL::FrameProfile GetDebugInfo();
#endif

    [[nodiscard]]
    inline const LLGL::RenderSystemPtr& GetLLGLContext() const noexcept {
        return m_context;
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

#if SGE_DEBUG
    [[nodiscard]]
    inline LLGL::RenderingDebugger* Debugger() const noexcept {
        return m_debugger;
    }
#endif

private:
    template <typename Container>
    std::size_t GetArraySize(const Container& container) const {
        return (container.size() * sizeof(typename Container::value_type));
    }

    template <typename T, std::size_t N>
    std::size_t GetArraySize(const T (&)[N]) const {
        return (N * sizeof(T));
    }

private:
    sge::DependencyGraph m_dependency_graph;

    std::unordered_map<uint32_t, LLGLResource<LLGL::SwapChain>> m_swapchain_map;
    std::unordered_map<uint32_t, GraphicsPipelineConfig> m_pipeline_configs;
    std::unordered_map<PipelineConfigKey, LLGL::PipelineState*> m_pipeline_states;

    LLGL::RenderSystemPtr m_context = nullptr;
    
#if SGE_DEBUG
    LLGL::RenderingDebugger* m_debugger = nullptr;
#endif

    LLGL::RenderTarget* m_current_target = nullptr;

    uint32_t m_pipeline_config_index = 0;
    uint32_t m_texture_index = 0;

    sge::RenderBackend m_backend;
};



};

#endif