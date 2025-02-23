#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include <GLFW/glfw3.h>

#include <LLGL/Shader.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/PipelineState.h>

#include "../types/backend.hpp"
#include "../types/texture.hpp"
#include "../types/shader_path.hpp"
#include "../types/shader_def.hpp"

#include "custom_surface.hpp"
#include "batch.hpp"
#include "camera.hpp"
#include "types.hpp"

#include "../defines.hpp"

_SGE_BEGIN

namespace renderer {

struct ALIGN(16) ProjectionsUniform {
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

class Renderer {
public:
    bool InitEngine(types::RenderBackend backend);
    bool Init(GLFWwindow* window, const LLGL::Extent2D& resolution, bool vsync, bool fullscreen);

    void Begin(const Camera& camera);
    void BeginMainPass(const LLGL::ClearValue& clear_color, long flags = LLGL::ClearFlags::Color);
    void EndMainPass();
    void End();

    void PrepareBatch(batch::Batch& batch);
    void UploadBatchData();
    void RenderBatch(batch::Batch& batch);

    types::Sampler CreateSampler(const LLGL::SamplerDescriptor& descriptor);

    types::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const types::Sampler& sampler, const void* data, bool generate_mip_maps = false);

    types::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const types::Sampler& sampler, const uint8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::UInt8, width, height, layers, sampler, data, generate_mip_maps);
    }
    types::Texture CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, uint32_t width, uint32_t height, uint32_t layers, const types::Sampler& sampler, const int8_t* data, bool generate_mip_maps = false) {
        return CreateTexture(type, image_format, LLGL::DataType::Int8, width, height, layers, sampler, data, generate_mip_maps);
    }

    LLGL::Shader* LoadShader(const types::ShaderPath& shader_path, const std::vector<types::ShaderDef>& shader_defs = {}, const std::vector<LLGL::VertexAttribute>& vertex_attributes = {});

    void ResizeBuffers(LLGL::Extent2D size);

    void Terminate();

    #if DEBUG
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
    [[nodiscard]] inline types::RenderBackend Backend() const { return m_backend; };

#if DEBUG
    [[nodiscard]] inline LLGL::RenderingDebugger* Debugger() const { return m_debugger; }
#endif

private:
    void InitSpriteBatchPipeline();
    void InitGlyphBatchPipeline();
    void InitNinepatchBatchPipeline();
    void InitShapeBatchPipeline();
    void InitBlurPipeline();
    void InitBlitPipeline();

    void SortBatchDrawCommands(batch::Batch& batch);
    void UpdateBatchBuffers(batch::Batch& batch, size_t begin = 0);
    void ApplyBatchDrawCommands(batch::Batch& batch);

    void RunBlurPass(uint32_t blur_radius);

    inline void UpdateBuffer(LLGL::Buffer* buffer, void* data, size_t length, size_t offset = 0) {
        static constexpr size_t SIZE = (1 << 16) - 1;

        const uint8_t* const src = static_cast<const uint8_t*>(data);

        while (offset < length) {
            const size_t len = std::min(offset + SIZE, length) - offset;
            m_command_buffer->UpdateBuffer(*buffer, offset, src + offset, len);
            offset += SIZE;
        }
    }

private:
    struct {
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::PipelineState* pipeline_depth = nullptr;
        LLGL::PipelineState* pipeline_ui = nullptr;

        SpriteInstance* buffer = nullptr;
        SpriteInstance* buffer_ptr = nullptr;

        LLGL::Buffer* vertex_buffer = nullptr;
        LLGL::Buffer* instance_buffer = nullptr;
        LLGL::BufferArray* buffer_array = nullptr;
    } m_sprite_batch_data;

    struct {
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::PipelineState* pipeline_ui = nullptr;

        GlyphInstance* buffer = nullptr;
        GlyphInstance* buffer_ptr = nullptr;

        LLGL::Buffer* vertex_buffer = nullptr;
        LLGL::Buffer* instance_buffer = nullptr;
        LLGL::BufferArray* buffer_array = nullptr;
    } m_glyph_batch_data;

    struct NinePatchBatchData {
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::PipelineState* pipeline_ui = nullptr;

        NinePatchInstance* buffer = nullptr;
        NinePatchInstance* buffer_ptr = nullptr;

        LLGL::Buffer* vertex_buffer = nullptr;
        LLGL::Buffer* instance_buffer = nullptr;
        LLGL::BufferArray* buffer_array = nullptr;
    } m_ninepatch_batch_data;

    struct ShapeBatchData {
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::PipelineState* pipeline_ui = nullptr;
        LLGL::PipelineState* pipeline_stencil_write = nullptr;

        ShapeInstance* buffer = nullptr;
        ShapeInstance* buffer_ptr = nullptr;

        LLGL::Buffer* vertex_buffer = nullptr;
        LLGL::Buffer* instance_buffer = nullptr;
        LLGL::BufferArray* buffer_array = nullptr;
    } m_shape_batch_data;

    LLGL::RenderSystemPtr m_context = nullptr;
    std::shared_ptr<CustomSurface> m_surface = nullptr;

    LLGL::SwapChain* m_swap_chain = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;
    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::Buffer* m_constant_buffer = nullptr;

    LLGL::RenderPass* m_pass = nullptr;

    LLGL::PipelineState* m_blit_pipeline = nullptr;
    LLGL::PipelineState* m_blit_stencil_pipeline = nullptr;
    LLGL::PipelineState* m_blit_blend_pipeline = nullptr;
    LLGL::Buffer* m_blit_vertex_buffer = nullptr;

    LLGL::PipelineState* m_blur_pipeline = nullptr;
    LLGL::Buffer* m_blur_vertex_buffer = nullptr;

    LLGL::Sampler* m_sampler_linear = nullptr;
    LLGL::Sampler* m_sampler_nearest = nullptr;

    LLGL::Texture* m_blur_texture_a = nullptr;
    LLGL::Texture* m_blur_texture_b = nullptr;
    LLGL::Texture* m_blur_texture_c = nullptr;
    LLGL::Texture* m_blur_stencil = nullptr;
    LLGL::RenderTarget* m_blur_framebuffer_a = nullptr;
    LLGL::RenderTarget* m_blur_framebuffer_b = nullptr;
    LLGL::RenderTarget* m_blur_framebuffer_c = nullptr;

#if DEBUG
    LLGL::RenderingDebugger* m_debugger = nullptr;
#endif

    LLGL::RenderTarget* m_current_framebuffer = nullptr;
    LLGL::RenderPass* m_current_pass = nullptr;

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

    types::RenderBackend m_backend;
};

}

_SGE_END

#endif