#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include <GLFW/glfw3.h>

#include <LLGL/Shader.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/RenderSystemFlags.h>

#include <SGE/types/backend.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/renderer/custom_surface.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/defines.hpp>

_SGE_BEGIN

template <typename T>
class BatchData {
public:
    std::vector<LLGL::VertexAttribute> Init(const sge::Renderer& renderer, uint32_t size, const sge::Attributes& vertex_attributes, const sge::Attributes& instance_attributes);
    void Destroy(const LLGL::RenderSystemPtr& context);

    inline void Update(LLGL::CommandBuffer* command_buffer) {
        command_buffer->UpdateBuffer(*m_instance_buffer, 0, m_buffer, m_count * sizeof(T));
    }

    inline void Reset() {
        m_count = 0;
        m_buffer_ptr = m_buffer;
    }

    [[nodiscard]]
    inline T* GetBufferAndAdvance() {
        m_count++;
        return m_buffer_ptr++;
    }

    [[nodiscard]]
    inline LLGL::BufferArray* GetBufferArray() const {
        return m_buffer_array;
    }

    [[nodiscard]]
    inline uint32_t Count() const {
        return m_count;
    }

private:
    T* m_buffer = nullptr;
    T* m_buffer_ptr = nullptr;

    LLGL::Buffer* m_vertex_buffer = nullptr;
    LLGL::Buffer* m_instance_buffer = nullptr;
    LLGL::BufferArray* m_buffer_array = nullptr;

    uint32_t m_count = 0;
};

struct SpriteBatchData : public BatchData<SpriteInstance> {
    LLGL::PipelineState* pipeline_additive = nullptr;
    LLGL::PipelineState* pipeline_alpha_blend = nullptr;
    LLGL::PipelineState* pipeline_opaque = nullptr;
    LLGL::PipelineState* pipeline_premultiplied_alpha = nullptr;

    LLGL::PipelineState* pipeline_depth_additive = nullptr;
    LLGL::PipelineState* pipeline_depth_alpha_blend = nullptr;
    LLGL::PipelineState* pipeline_depth_opaque = nullptr;
    LLGL::PipelineState* pipeline_depth_premultiplied_alpha = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        BatchData::Destroy(context);
        SGE_RESOURCE_RELEASE(pipeline_additive);
        SGE_RESOURCE_RELEASE(pipeline_alpha_blend);
        SGE_RESOURCE_RELEASE(pipeline_opaque);
        SGE_RESOURCE_RELEASE(pipeline_premultiplied_alpha);
        SGE_RESOURCE_RELEASE(pipeline_depth_additive);
        SGE_RESOURCE_RELEASE(pipeline_depth_alpha_blend);
        SGE_RESOURCE_RELEASE(pipeline_depth_opaque);
        SGE_RESOURCE_RELEASE(pipeline_depth_premultiplied_alpha);
    }
};

struct GlyphBatchData : public BatchData<GlyphInstance> {
    LLGL::PipelineState* pipeline = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        BatchData::Destroy(context);
        SGE_RESOURCE_RELEASE(pipeline);
    }
};

struct NinePatchBatchData : public BatchData<NinePatchInstance> {
    LLGL::PipelineState* pipeline = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        BatchData::Destroy(context);
        SGE_RESOURCE_RELEASE(pipeline);
    }
};

struct ShapeBatchData : public BatchData<ShapeInstance> {
    LLGL::PipelineState* pipeline = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        BatchData::Destroy(context);
        SGE_RESOURCE_RELEASE(pipeline);
    }
};

struct LineBatchData : public BatchData<LineInstance> {
    LLGL::PipelineState* pipeline = nullptr;

    void Destroy(const LLGL::RenderSystemPtr& context) {
        BatchData::Destroy(context);
        SGE_RESOURCE_RELEASE(pipeline);
    }
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
    bool Init(GLFWwindow* window, const LLGL::Extent2D& resolution, const WindowSettings& settings);

    void Begin(const sge::Camera& camera);

    inline void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
    }

    void BeginPassWithViewport(LLGL::RenderTarget& target, const LLGL::Viewport& viewport);

    inline void BeginPass(LLGL::RenderTarget& target) {
        BeginPassWithViewport(target, target.GetResolution());
    }

    inline void BeginMainPass() {
        BeginPass(*m_swap_chain);
    }

    inline void EndPass() {
        m_command_buffer->EndRenderPass();
    }

    inline void SetScissor(const LLGL::Scissor& scissor) {
        m_command_buffer->SetScissor(scissor);
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
    inline LLGL::Buffer* CreateVertexBuffer(const Container& vertices, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) const
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(GetArraySize(vertices), vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, &vertices[0]);
    }

    inline LLGL::Buffer* CreateVertexBuffer(size_t size, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) const
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, nullptr);
    }

    inline LLGL::Buffer* CreateVertexBufferInit(size_t size, const void* data, const LLGL::VertexFormat& vertexFormat, const char* debug_name = nullptr) const
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::VertexBufferDesc(size, vertexFormat);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, data);
    }

    template <typename Container>
    inline LLGL::Buffer* CreateIndexBuffer(const Container& indices, const LLGL::Format format, const char* debug_name = nullptr) const
    {
        LLGL::BufferDescriptor bufferDesc = LLGL::IndexBufferDesc(GetArraySize(indices), format);
        bufferDesc.debugName = debug_name;
        return m_context->CreateBuffer(bufferDesc, &indices[0]);
    }

    inline LLGL::Buffer* CreateConstantBuffer(const size_t size, const char* debug_name = nullptr) const
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

    [[nodiscard]] inline const LLGL::RendererInfo& GetRendererInfo() const { return m_context->GetRendererInfo(); }
    [[nodiscard]] inline const LLGL::RenderingCapabilities& GetRenderingCaps() const { return m_context->GetRenderingCaps(); }

private:
    SpriteBatchData InitSpriteBatchPipeline();
    NinePatchBatchData InitNinepatchBatchPipeline();
    GlyphBatchData InitGlyphBatchPipeline();
    ShapeBatchData InitShapeBatchPipeline();
    LineBatchData InitLineBatchPipeline();

    LLGL::PipelineCache* ReadPipelineCache(const std::string& name, bool& hasInitialCache);
    void SavePipelineCache(const std::string& name, LLGL::PipelineCache* pipelineCache);

    void SortBatchDrawCommands(sge::Batch& batch);
    void UpdateBatchBuffers(sge::Batch& batch, size_t begin = 0);
    void ApplyBatchDrawCommands(sge::Batch& batch);

private:
    SpriteBatchData m_sprite_batch_data;
    GlyphBatchData m_glyph_batch_data;
    NinePatchBatchData m_ninepatch_batch_data;
    ShapeBatchData m_shape_batch_data;
    LineBatchData m_line_batch_data;

    std::string m_cache_pipeline_dir;

    LLGL::RenderSystemPtr m_context = nullptr;
    std::shared_ptr<CustomSurface> m_surface = nullptr;

    LLGL::SwapChain* m_swap_chain = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;
    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::Buffer* m_constant_buffer = nullptr;

#if SGE_DEBUG
    LLGL::RenderingDebugger* m_debugger = nullptr;
#endif

    glm::uvec2 m_viewport = glm::uvec2(0);

    uint32_t m_texture_index = 0;

    size_t m_batch_instance_count = 0;

    sge::RenderBackend m_backend;

    bool m_cache_pipelines = true;
};

_SGE_END

#endif
