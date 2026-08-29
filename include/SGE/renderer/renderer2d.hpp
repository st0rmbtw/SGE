#ifndef SGE_RENDERER_RENDERER_2D_HPP_
#define SGE_RENDERER_RENDERER_2D_HPP_

#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/types/transform.hpp>

#include "SGE/renderer/batch.hpp"
#include "renderer.hpp"

#include <SGE/types/path.hpp>

namespace sge {

struct BatchSubmission {
    std::shared_ptr<IBatch> batch; // Needed to hold the reference to the batch so it doesn't get released
    const uint8_t* instancesData;
    size_t instanceStride;
    LLGL::Resource* const* dynamicBindings;
    std::span<const DrawCommand> drawCommands;
    size_t drawCommandsOffset = 0;
};

struct BufferPoolEntry {
    VertexBufferPool buffer;
    bool occupied;
};

struct BatchData {
    sge::internal::BatchState state;
    uint32_t offset;
    bool allTheSame;
};

class Renderer2D : public Renderer {
public:
    explicit Renderer2D(const std::shared_ptr<RenderContext>& context);

    void SubmitBatch(std::shared_ptr<IBatch> batch);
    void FlushBatches();

    template <typename... Args>
    void SubmitBatches(Args&... batches) {
        (SubmitBatch(batches), ...);
    }

    void DrawPath(const sge::Path& path, const sge::LinearRgba& color = sge::color::WHITE, const sge::Transform& transform = sge::Transform());

private:
    void InitVectorPipeline();

private:
    LLGL::VertexFormat m_vector_vertex_format;
    Ref<LLGL::Shader> m_vector_vertex_shader;
    Ref<LLGL::Shader> m_vector_fragment_shader;
    Ref<LLGL::PipelineLayout> m_vector_stencil_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_stencil_pipeline;
    Ref<LLGL::PipelineLayout> m_vector_cover_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_cover_pipeline;
    Ref<LLGL::Buffer> m_vector_vertex_buffer;
    Ref<LLGL::Buffer> m_vector_path_data_buffer;

    std::vector<size_t> m_sort_indices;
    std::vector<BatchSubmission> m_submissions;
    std::vector<BatchData> m_batch_data;

    bool m_vector_pipeline_initialized = false;
};

} // namespace sge

#endif // SGE_RENDERER_RENDERER_2D_HPP_