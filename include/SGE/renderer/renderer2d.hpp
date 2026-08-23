#ifndef _SGE_RENDERER_2D_HPP_
#define _SGE_RENDERER_2D_HPP_

#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/handle.hpp>
#include <SGE/types/transform.hpp>

#include "SGE/renderer/batch.hpp"
#include "renderer.hpp"

#include <SGE/types/path.hpp>

namespace sge {

struct Binding {
    uint32_t index;
    LLGL::Resource* resource;
};

struct BatchSubmission {
    LLGL::PipelineState* pipeline;
    LLGL::BufferArray* buffer_array;
    LLGL::Resource* const* resources;
    const DrawCommand* draw_commands;
    size_t draw_commands_count;
    size_t draw_commands_offset = 0;
    uint32_t vertex_count = 0;
};

struct BufferPoolEntry {
    VertexBufferPool buffer;
    bool occupied;
};

struct BatchData {
    sge::internal::BatchState state;
    uint32_t offset;
};

class Renderer2D : public Renderer {
    friend class Batch;
public:
    explicit Renderer2D(const std::shared_ptr<RenderContext>& context);

    void Begin() {
        m_batch_instance_count = 0;
        Renderer::Begin();
    }

    void PrepareBatch(sge::Batch& batch) {}
    void RenderBatch(sge::Batch& batch) {}
    void UploadBatchData() {}

    template <Batchable T>
    void SubmitBatch(T& batch) {
        SubmitBatchRaw(
            &m_context->GetOrCreatePipeline(batch.GetPipeline()),
            batch.GetVertexBuffer(),
            batch.GetInstanceBuffer(),
            batch.BufferArray(),
            batch.GetDrawCommands(),
            batch.GetResources(),
            batch.GetVertexCount(),
            batch.GetInstanceData()
        );
    }

    void SubmitBatchRaw(
        LLGL::PipelineState* pipeline,
        LLGL::Buffer* vertexBuffer,
        sge::VertexBufferPool& instanceBuffer,
        sge::Unique<LLGL::BufferArray>& bufferArray,
        std::vector<DrawCommand>& drawCommands,
        LLGL::Resource* const* resources,
        uint32_t vertex_count,
        void* instanceData
    );

    void FlushBatches();

    template <typename... Args>
    void PrepareAndUpload(Args&... batches) {
        (PrepareBatch(batches), ...);
        UploadBatchData();
    }

    inline std::unique_ptr<sge::Batch> CreateBatch(const sge::BatchDesc& desc = {}) {
        return std::make_unique<sge::Batch>(*this, desc);
    }

    void DestroyBatch(sge::Batch& batch) {}

    void DrawPath(const sge::Path& path, const sge::LinearRgba& color = sge::color::WHITE, const sge::Transform& transform = sge::Transform());

private:
    void InitVectorPipeline();

private:
    Ref<LLGL::Shader> m_glyph_sdf_vertex_shader;
    Ref<LLGL::Shader> m_ninepatch_vertex_shader;

    Ref<LLGL::Shader> m_sprite_default_fragment_shader;
    Ref<LLGL::Shader> m_glyph_sdf_default_fragment_shader;

    Ref<LLGL::Shader> m_glyph_vector_fragment_shader;

    LLGL::VertexFormat m_vector_vertex_format;

    Ref<LLGL::Shader> m_vector_vertex_shader;
    Ref<LLGL::Shader> m_vector_fragment_shader;
    Ref<LLGL::PipelineLayout> m_vector_stencil_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_stencil_pipeline;
    Ref<LLGL::PipelineLayout> m_vector_cover_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_cover_pipeline;
    Ref<LLGL::Buffer> m_vector_vertex_buffer;
    Ref<LLGL::Buffer> m_vector_path_data_buffer;

    std::vector<size_t> m_indices;

    std::vector<BatchSubmission> m_submissions;
    std::vector<BatchData> m_batch_data;

    uint32_t m_batch_path_total_count = 0;

    uint32_t m_batch_instance_count = 0;

    bool m_vector_pipeline_initialized = false;
};

} // namespace sge

#endif // _SGE_RENDERER_2D_HPP_