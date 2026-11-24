#ifndef SGE_RENDERER_RENDERER_2D_HPP_
#define SGE_RENDERER_RENDERER_2D_HPP_

#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/path.hpp>


namespace sge::internal {
    struct BufferPoolEntry {
        BufferPool buffer;
        bool occupied;
    };

    struct SortItem {
        uint64_t key;
        uint32_t index;
    };

    struct SortScratch {
        std::vector<BatchRun> runs;
        std::vector<uint8_t>  instances;
    };

    struct BatchSubmission {
        std::shared_ptr<IBatch>   batch; // Needed to hold the reference to the batch so it doesn't get released
        std::span<const BatchRun> runsSpan;
        const uint8_t*            instancesBytes;
        const DrawStateTable*     states;
        size_t                    instanceStride;
        size_t                    cursor = 0; // Next not processed draw
    };
}

namespace sge {

class IBatch;

class Renderer2D : public Renderer {
public:
    explicit Renderer2D(const std::shared_ptr<RenderContext>& context);

    void SubmitBatch(std::shared_ptr<IBatch> batch);
    void FlushBatches();

    template <typename... Args>
    void SubmitBatches(Args&... batches) {
        (SubmitBatch(batches), ...);
    }

    // void DrawPath(const sge::vector::VectorShape& path, const sge::LinearRgba& color = sge::color::WHITE, const sge::Transform& transform = sge::Transform());

private:
    void InitVectorPipeline();

    void OnPassEnded() override {
        m_scratch_used = 0;
        for (const internal::BatchSubmission& s : m_submissions) {
            s.batch->Reset();
        }
    }

private:
    Ref<LLGL::Shader> m_vector_vertex_shader;
    Ref<LLGL::Shader> m_vector_fragment_shader;
    Ref<LLGL::PipelineLayout> m_vector_stencil_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_stencil_pipeline;
    Ref<LLGL::PipelineLayout> m_vector_cover_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_cover_pipeline;
    Ref<LLGL::Buffer> m_vector_vertex_buffer;
    Ref<LLGL::Buffer> m_vector_path_data_buffer;

    std::vector<sge::internal::SortItem>    m_sort_items;
    std::vector<sge::internal::SortItem>    m_sort_tmp;
    std::vector<sge::internal::SortScratch> m_scratch_pool;
    std::vector<sge::internal::BatchSubmission> m_submissions;

    size_t m_scratch_used = 0;

    bool m_vector_pipeline_initialized = false;
};

} // namespace sge

#endif // SGE_RENDERER_RENDERER_2D_HPP_
