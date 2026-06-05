#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include <GLFW/glfw3.h>

#include <LLGL/CommandBufferFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/Shader.h>
#include <LLGL/Utils/Utility.h>

#include <SGE/defines.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/window_settings.hpp>

#include <memory>

namespace sge {

template <typename T>
class BatchData {
public:
    BatchData() = default;
    
    BatchData(BatchData&&) = default;
    BatchData& operator=(BatchData&&) = default;

    void Init(sge::RenderContext& context, uint32_t count, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format);

    void CreateDynamicBuffers(sge::RenderContext& context, uint32_t count);

    inline void ResizeBuffersIfNeeded(sge::RenderContext& context, uint32_t count) {
        if (count < m_max_count) return;
        CreateDynamicBuffers(context, count + 2500);
    }

    inline void Update(LLGL::CommandBuffer& command_buffer, uint32_t offset, const T* buffer, uint32_t count) {
        UpdateBufferChunked(command_buffer, *m_instance_buffer, offset * sizeof(T), buffer, count * sizeof(T));
    }

    [[nodiscard]]
    inline const sge::Unique<LLGL::BufferArray>& GetBufferArray() const {
        return m_buffer_array;
    }

    [[nodiscard]]
    inline uint32_t MaxCount() const {
        return m_max_count;
    }

private:
    sge::Unique<LLGL::Buffer> m_vertex_buffer;
    sge::Unique<LLGL::Buffer> m_instance_buffer;
    sge::Unique<LLGL::BufferArray> m_buffer_array;
    
    LLGL::VertexFormat m_instance_format;

    uint32_t m_max_count = 0;
};

struct SGE_ALIGN(16) GlobalUniforms {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::mat4 nonscale_view_projection_matrix;
    glm::mat4 nonscale_projection_matrix;
    glm::vec2 camera_position;
    glm::uvec2 window_size;
};

class Renderer {
    friend class Batch;

public:
    explicit Renderer(const std::shared_ptr<RenderContext>& context);

    void Begin();
    void BeginPass(LLGL::RenderTarget& target, const Camera& camera);
    inline void BeginPass(const std::shared_ptr<GlfwWindow>& window, const Camera& camera) {
        BeginPass(m_context->GetOrCreateSwapChain(window), camera);
    }

    inline void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
    }

    inline void EndPass() {
        m_context->PopRenderTarget();
        m_command_buffer->EndRenderPass();
    }

    inline void SetScissor(const LLGL::Scissor& scissor) {
        m_command_buffer->SetScissor(scissor);
    }

    void Present(const std::shared_ptr<GlfwWindow>& window) {
        m_context->Present(*window);
    }

    void End();

    void PrepareBatch(sge::Batch& batch);
    void RenderBatch(sge::Batch& batch);

    [[nodiscard]]
    inline LLGL::CommandBuffer* CommandBuffer() const noexcept {
        return m_command_buffer;
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* CommandQueue() const noexcept {
        return m_command_queue;
    }

    [[nodiscard]]
    inline const Unique<LLGL::Buffer>& GlobalUniformBuffer() const noexcept {
        return m_uniform_buffer;
    }

    [[nodiscard]]
    const std::shared_ptr<RenderContext>& GetRenderContext() const noexcept {
        return m_context;
    }

    inline std::unique_ptr<sge::Batch> CreateBatch(const sge::BatchDesc& desc = {}) {
        return std::make_unique<sge::Batch>(*this, desc);
    }

    void DestroyBatch(sge::Batch& batch);

private:
    SpriteBatchPipeline CreateSpriteBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader = Ref<LLGL::Shader>());
    Handle<LLGL::PipelineState> CreateNinepatchBatchPipeline(bool enable_scissor);
    Handle<LLGL::PipelineState> CreateGlyphBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader = Ref<LLGL::Shader>());
    Handle<LLGL::PipelineState> CreateShapeBatchPipeline(bool enable_scissor);
    Handle<LLGL::PipelineState> CreateLineBatchPipeline(bool enable_scissor);

    BatchData<SpriteInstance> InitSpriteBatchData();
    BatchData<NinePatchInstance> InitNinepatchBatchData();
    BatchData<GlyphInstance> InitGlyphBatchData();
    BatchData<ShapeInstance> InitShapeBatchData();
    BatchData<LineInstance> InitLineBatchData();

    void SortBatchDrawCommands(sge::Batch& batch);
    void UpdateBatchBuffers(sge::Batch& batch, size_t begin = 0);
    void ApplyBatchDrawCommands(sge::Batch& batch);
    void UploadBatchData(sge::Batch& batch);

private:
    BatchData<SpriteInstance> m_sprite_batch_data;
    BatchData<GlyphInstance> m_glyph_batch_data;
    BatchData<NinePatchInstance> m_ninepatch_batch_data;
    BatchData<ShapeInstance> m_shape_batch_data;
    BatchData<LineInstance> m_line_batch_data;

    Unique<LLGL::Buffer> m_uniform_buffer;
    
    std::shared_ptr<RenderContext> m_context;
    
    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;

    Ref<LLGL::Shader> m_sprite_vertex_shader;
    Ref<LLGL::Shader> m_glyph_vertex_shader;
    Ref<LLGL::Shader> m_ninepatch_vertex_shader;

    Ref<LLGL::Shader> m_sprite_default_fragment_shader;
    Ref<LLGL::Shader> m_glyph_default_fragment_shader;

    LLGL::Extent2D m_viewport = LLGL::Extent2D(0, 0);

    uint32_t m_batch_instance_count = 0;

};

} // namespace sge

#endif
