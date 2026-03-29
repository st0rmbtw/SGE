#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include "LLGL/PipelineStateFlags.h"
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
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/defines.hpp>
#include <SGE/utils/llgl.hpp>
#include <memory>

namespace sge {

template <typename T>
class BatchData {
public:
    void Init(const std::shared_ptr<sge::RenderContext>& context, uint32_t size, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format);
    void Destroy(const LLGL::RenderSystemPtr& context);

    inline void Update(LLGL::CommandBuffer& command_buffer) {
        command_buffer.UpdateBuffer(*m_instance_buffer, 0, m_buffer, m_count * sizeof(T));
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
    inline LLGL::BufferArray& GetBufferArray() const {
        return *m_buffer_array;
    }

    [[nodiscard]]
    inline uint32_t Count() const {
        return m_count;
    }

private:
    T* m_buffer = nullptr;
    T* m_buffer_ptr = nullptr;

    LLGLResource<LLGL::Buffer> m_vertex_buffer = nullptr;
    LLGLResource<LLGL::Buffer> m_instance_buffer = nullptr;
    LLGLResource<LLGL::BufferArray> m_buffer_array = nullptr;

    uint32_t m_count = 0;
};

struct SGE_ALIGN(16) GlobalUniforms {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 nonscale_view_projection_matrix;
    glm::mat4 nonscale_projection_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::vec2 camera_position;
    glm::vec2 window_size;
};

class Renderer {
    friend class Batch;

public:
    Renderer(const std::shared_ptr<RenderContext>& context);
    ~Renderer();

    void Begin();
    void BeginPass(LLGL::RenderTarget& target, const Camera& camera);
    inline void BeginPass(const std::shared_ptr<GlfwWindow>& window, const Camera& camera) {
        BeginPass(m_context->GetOrCreateSwapChain(window), camera);
    }

    inline void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
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

    [[nodiscard]]
    inline LLGL::CommandBuffer* CommandBuffer() const noexcept {
        return m_command_buffer.get();
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* CommandQueue() const noexcept {
        return m_command_queue.get();
    }

    [[nodiscard]]
    inline LLGL::Buffer* GlobalUniformBuffer() const noexcept {
        return m_constant_buffer.get();
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
    SpriteBatchPipeline CreateSpriteBatchPipeline(bool enable_scissor, LLGL::Shader* fragment_shader = nullptr);
    uint32_t CreateNinepatchBatchPipeline(bool enable_scissor);
    uint32_t CreateGlyphBatchPipeline(bool enable_scissor, LLGL::Shader* fragment_shader = nullptr);
    uint32_t CreateShapeBatchPipeline(bool enable_scissor);
    uint32_t CreateLineBatchPipeline(bool enable_scissor);

    BatchData<SpriteInstance> InitSpriteBatchData();
    BatchData<NinePatchInstance> InitNinepatchBatchData();
    BatchData<GlyphInstance> InitGlyphBatchData();
    BatchData<ShapeInstance> InitShapeBatchData();
    BatchData<LineInstance> InitLineBatchData();

    void SortBatchDrawCommands(sge::Batch& batch);
    void UpdateBatchBuffers(sge::Batch& batch, size_t begin = 0);
    void ApplyBatchDrawCommands(sge::Batch& batch);

private:
    BatchData<SpriteInstance> m_sprite_batch_data;
    BatchData<GlyphInstance> m_glyph_batch_data;
    BatchData<NinePatchInstance> m_ninepatch_batch_data;
    BatchData<ShapeInstance> m_shape_batch_data;
    BatchData<LineInstance> m_line_batch_data;

    std::shared_ptr<RenderContext> m_context;
    
    LLGLResource<LLGL::CommandBuffer> m_command_buffer = nullptr;
    LLGLResource<LLGL::CommandQueue> m_command_queue = nullptr;
    LLGLResource<LLGL::Buffer> m_constant_buffer = nullptr;

    LLGLResource<LLGL::Shader> m_sprite_vertex_shader = nullptr;
    LLGLResource<LLGL::Shader> m_glyph_vertex_shader = nullptr;
    LLGLResource<LLGL::Shader> m_ninepatch_vertex_shader = nullptr;

    LLGLResource<LLGL::Shader> m_sprite_default_fragment_shader = nullptr;
    LLGLResource<LLGL::Shader> m_glyph_default_fragment_shader = nullptr;

    LLGL::Extent2D m_viewport = LLGL::Extent2D(0, 0);

    size_t m_batch_instance_count = 0;

};

}

#endif
