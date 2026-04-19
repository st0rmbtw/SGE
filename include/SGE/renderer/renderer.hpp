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
#include <memory>

namespace sge {

template <typename T>
class BatchData {
public:
    BatchData() = default;
    ~BatchData() {
        if (m_buffer) free(m_buffer);
    }

    BatchData(BatchData&& other) noexcept {
        operator=(std::move(other));
    }

    BatchData(const BatchData& other) {
        operator=(other);
    }

    BatchData& operator=(BatchData&& other) noexcept {
        m_buffer = other.m_buffer;
        m_buffer_ptr = other.m_buffer_ptr;
        m_count = other.m_count;
        m_vertex_buffer = std::move(other.m_vertex_buffer);
        m_instance_buffer = std::move(other.m_instance_buffer);
        m_buffer_array = std::move(other.m_buffer_array);
        other.m_buffer = nullptr;
        other.m_buffer_ptr = nullptr;
        other.m_count = 0;
        return *this;
    }

    BatchData& operator=(const BatchData& other) = default;

    void Init(sge::RenderContext& context, uint32_t size, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format);

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
    inline const Ref<LLGL::BufferArray>& GetBufferArray() const {
        return m_buffer_array;
    }

    [[nodiscard]]
    inline uint32_t Count() const {
        return m_count;
    }

private:
    T* m_buffer = nullptr;
    T* m_buffer_ptr = nullptr;

    Ref<LLGL::Buffer> m_vertex_buffer;
    Ref<LLGL::Buffer> m_instance_buffer;
    Ref<LLGL::BufferArray> m_buffer_array;

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

    void Present(const std::shared_ptr<GlfwWindow>& window) {
        m_context->Present(window);
    }

    void End();

    void PrepareBatch(sge::Batch& batch);
    void UploadBatchData();
    void RenderBatch(sge::Batch& batch);

    [[nodiscard]]
    inline const Unique<LLGL::CommandBuffer>& CommandBuffer() const noexcept {
        return m_command_buffer;
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* CommandQueue() const noexcept {
        return m_command_queue;
    }

    [[nodiscard]]
    inline const Unique<LLGL::Buffer>& GlobalUniformBuffer() const noexcept {
        return m_constant_buffer;
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

private:
    BatchData<SpriteInstance> m_sprite_batch_data;
    BatchData<GlyphInstance> m_glyph_batch_data;
    BatchData<NinePatchInstance> m_ninepatch_batch_data;
    BatchData<ShapeInstance> m_shape_batch_data;
    BatchData<LineInstance> m_line_batch_data;

    std::shared_ptr<RenderContext> m_context;
    
    LLGL::CommandQueue* m_command_queue;
    Unique<LLGL::CommandBuffer> m_command_buffer;
    Unique<LLGL::Buffer> m_constant_buffer;

    Ref<LLGL::Shader> m_sprite_vertex_shader;
    Ref<LLGL::Shader> m_glyph_vertex_shader;
    Ref<LLGL::Shader> m_ninepatch_vertex_shader;

    Ref<LLGL::Shader> m_sprite_default_fragment_shader;
    Ref<LLGL::Shader> m_glyph_default_fragment_shader;

    LLGL::Extent2D m_viewport = LLGL::Extent2D(0, 0);

    size_t m_batch_instance_count = 0;

};

}

#endif
