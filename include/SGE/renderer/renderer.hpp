#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include "LLGL/RenderSystemFlags.h"
#include <GLFW/glfw3.h>

#include <LLGL/Shader.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/PipelineState.h>

#include <SGE/types/backend.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/renderer/custom_surface.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/defines.hpp>

_SGE_BEGIN

struct SpriteBatchData {
    LLGL::PipelineState* pipeline_additive = nullptr;
    LLGL::PipelineState* pipeline_alpha_blend = nullptr;
    LLGL::PipelineState* pipeline_opaque = nullptr;
    LLGL::PipelineState* pipeline_premultiplied_alpha = nullptr;

    LLGL::PipelineState* pipeline_depth_additive = nullptr;
    LLGL::PipelineState* pipeline_depth_alpha_blend = nullptr;
    LLGL::PipelineState* pipeline_depth_opaque = nullptr;
    LLGL::PipelineState* pipeline_depth_premultiplied_alpha = nullptr;

    sge::SpriteInstance* buffer = nullptr;
    sge::SpriteInstance* buffer_ptr = nullptr;

    LLGL::Buffer* vertex_buffer = nullptr;
    LLGL::Buffer* instance_buffer = nullptr;
    LLGL::BufferArray* buffer_array = nullptr;
};

struct GlyphBatchData {
    LLGL::PipelineState* pipeline = nullptr;

    sge::GlyphInstance* buffer = nullptr;
    sge::GlyphInstance* buffer_ptr = nullptr;

    LLGL::Buffer* vertex_buffer = nullptr;
    LLGL::Buffer* instance_buffer = nullptr;
    LLGL::BufferArray* buffer_array = nullptr;
};

struct NinePatchBatchData {
    LLGL::PipelineState* pipeline = nullptr;

    sge::NinePatchInstance* buffer = nullptr;
    sge::NinePatchInstance* buffer_ptr = nullptr;

    LLGL::Buffer* vertex_buffer = nullptr;
    LLGL::Buffer* instance_buffer = nullptr;
    LLGL::BufferArray* buffer_array = nullptr;
};

struct ShapeBatchData {
    LLGL::PipelineState* pipeline = nullptr;

    sge::ShapeInstance* buffer = nullptr;
    sge::ShapeInstance* buffer_ptr = nullptr;

    LLGL::Buffer* vertex_buffer = nullptr;
    LLGL::Buffer* instance_buffer = nullptr;
    LLGL::BufferArray* buffer_array = nullptr;
};

struct SGE_ALIGN(16) ProjectionsUniform {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 nonscale_view_projection_matrix;
    glm::mat4 nonscale_projection_matrix;
    glm::mat4 transform_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::vec2 camera_position;
    glm::vec2 window_size;
};

template <typename Container>
static inline std::size_t GetArraySize(const Container& container)
{
    return (container.size() * sizeof(typename Container::value_type));
}

template <typename T, std::size_t N>
static inline std::size_t GetArraySize(const T (&)[N])
{
    return (N * sizeof(T));
}

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

class Renderer {
public:
    bool InitEngine(sge::RenderBackend backend, bool cache_pipelines = true, const std::string& cache_dir_path = DEFAULT_CACHE_DIR);
    bool Init(GLFWwindow* window, const LLGL::Extent2D& resolution, bool vsync, bool fullscreen);

    void Begin(const sge::Camera& camera);

    void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
    }

    void BeginPassWithViewport(LLGL::RenderTarget& target, const LLGL::Viewport& viewport);

    void BeginPass(LLGL::RenderTarget& target) {
        BeginPassWithViewport(target, target.GetResolution());
    }

    void BeginMainPass() {
        BeginPass(*m_swap_chain);
    }

    void EndPass() {
        m_command_buffer->EndRenderPass();
    }

    void End();

    void PrepareBatch(sge::Batch& batch);
    void UploadBatchData();
    void RenderBatch(sge::Batch& batch);

    sge::Sampler CreateSampler(const LLGL::SamplerDescriptor& descriptor);

    sge::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const void* data, bool generate_mip_maps = false);

    sge::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const uint8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::UInt8, width, height, layers, sampler, data, generate_mip_maps);
    }
    sge::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const sge::Sampler& sampler, const int8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::Int8, width, height, layers, sampler, data, generate_mip_maps);
    }

    sge::Texture CopyTextureWithSampler(const sge::Texture& texture, const sge::Sampler& sampler) {
        return Texture(m_texture_index++, sampler, texture.size(), texture.internal());
    }

    LLGL::Shader* LoadShader(const sge::ShaderPath& shader_path, const std::vector<sge::ShaderDef>& shader_defs = {}, const std::vector<LLGL::VertexAttribute>& vertex_attributes = {});

    void ResizeBuffers(LLGL::Extent2D size);

    void Terminate();

    #if SGE_DEBUG
        void PrintDebugInfo();
    #endif

    template <typename Container>
    inline LLGL::Buffer* CreateVertexBuffer(const Container& vertices, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(GetArraySize(vertices), vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, &vertices[0]);
    }

    inline LLGL::Buffer* CreateVertexBuffer(size_t size, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, nullptr);
    }

    inline LLGL::Buffer* CreateVertexBufferInit(size_t size, const void* data, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, data);
    }

    template <typename Container>
    inline LLGL::Buffer* CreateIndexBuffer(const Container& indices, const LLGL::Format format, const char* debug_name = nullptr)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::IndexBufferDesc(GetArraySize(indices), format);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, &indices[0]);
    }

    inline LLGL::Buffer* CreateConstantBuffer(const size_t size, const char* debug_name = nullptr)
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::ConstantBufferDesc(size);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc);
    }

    [[nodiscard]] inline const LLGL::RenderSystemPtr& Context() const { return m_context; }
    [[nodiscard]] inline LLGL::SwapChain* SwapChain() const { return m_swap_chain; };
    [[nodiscard]] inline LLGL::CommandBuffer* CommandBuffer() const { return m_command_buffer; };
    [[nodiscard]] inline LLGL::CommandQueue* CommandQueue() const { return m_command_queue; };
    [[nodiscard]] inline const std::shared_ptr<CustomSurface>& Surface() const { return m_surface; };
    [[nodiscard]] inline LLGL::Buffer* GlobalUniformBuffer() const { return m_constant_buffer; };
    [[nodiscard]] inline sge::RenderBackend Backend() const { return m_backend; };

#if SGE_DEBUG
    [[nodiscard]] inline LLGL::RenderingDebugger* Debugger() const { return m_debugger; }
#endif

    [[nodiscard]] inline const LLGL::RendererInfo& GetRendererInfo() { return m_context->GetRendererInfo(); }
    [[nodiscard]] inline const LLGL::RenderingCapabilities& GetRenderingCaps() { return m_context->GetRenderingCaps(); }

private:
    SpriteBatchData InitSpriteBatchPipeline();
    NinePatchBatchData InitNinepatchBatchPipeline();
    GlyphBatchData InitGlyphBatchPipeline();
    ShapeBatchData InitShapeBatchPipeline();

    LLGL::PipelineCache* ReadPipelineCache(const std::string& name, bool& hasInitialCache);
    void SavePipelineCache(const std::string& name, LLGL::PipelineCache* pipelineCache);

    void SortBatchDrawCommands(sge::Batch& batch);
    void UpdateBatchBuffers(sge::Batch& batch, size_t begin = 0);
    void ApplyBatchDrawCommands(sge::Batch& batch);

    inline void UpdateBuffer(LLGL::Buffer* buffer, void* data, uint64_t length, uint64_t offset = 0) {
        const char* src = static_cast<char*>(data);
        m_command_buffer->UpdateBuffer(*buffer, offset, src + offset, length);
    }

private:
    SpriteBatchData m_sprite_batch_data;
    GlyphBatchData m_glyph_batch_data;
    NinePatchBatchData m_ninepatch_batch_data;
    ShapeBatchData m_shape_batch_data;

    std::string m_cache_pipeline_dir;

    LLGL::RenderSystemPtr m_context = nullptr;
    std::shared_ptr<CustomSurface> m_surface = nullptr;

    LLGL::SwapChain* m_swap_chain = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;
    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::Buffer* m_constant_buffer = nullptr;

    LLGL::RenderPass* m_pass = nullptr;

#if SGE_DEBUG
    LLGL::RenderingDebugger* m_debugger = nullptr;
#endif

    glm::uvec2 m_viewport = glm::uvec2(0);

    uint32_t m_texture_index = 0;

    size_t m_batch_instance_count = 0;

    size_t m_sprite_instance_size = 0;
    size_t m_glyph_instance_size = 0;
    size_t m_ninepatch_instance_size = 0;
    size_t m_shape_instance_size = 0;

    size_t m_sprite_instance_count = 0;
    size_t m_glyph_instance_count = 0;
    size_t m_ninepatch_instance_count = 0;
    size_t m_shape_instance_count = 0;

    sge::RenderBackend m_backend;

    bool m_cache_pipelines = true;
};

_SGE_END

#endif