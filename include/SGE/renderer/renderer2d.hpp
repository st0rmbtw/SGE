#ifndef _SGE_RENDERER_2D_HPP_
#define _SGE_RENDERER_2D_HPP_

#include "SGE/types/transform.hpp"
#include "renderer.hpp"

#include <SGE/types/path.hpp>

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

    inline void Update(LLGL::CommandBuffer& command_buffer) {
        UpdateBufferChunked(command_buffer, *m_instance_buffer, 0, m_buffer.data(), m_count * sizeof(T));
    }

    inline void Reset() {
        m_count = 0;
        m_buffer_ptr = m_buffer.data();
    }

    [[nodiscard]]
    inline T* GetBufferAndAdvance() {
        m_count++;
        return m_buffer_ptr++;
    }

    [[nodiscard]]
    inline const sge::Unique<LLGL::BufferArray>& GetBufferArray() const {
        return m_buffer_array;
    }

    [[nodiscard]]
    inline uint32_t Count() const {
        return m_count;
    }

    [[nodiscard]]
    inline uint32_t MaxCount() const {
        return m_max_count;
    }

private:
    sge::HeapArray<T> m_buffer;
    T* m_buffer_ptr = nullptr;

    sge::Unique<LLGL::Buffer> m_vertex_buffer;
    sge::Unique<LLGL::Buffer> m_instance_buffer;
    sge::Unique<LLGL::BufferArray> m_buffer_array;
    
    LLGL::VertexFormat m_instance_format;

    uint32_t m_count = 0;
    uint32_t m_max_count = 0;
};

class Renderer2D : public Renderer {
    friend class Batch;
public:
    explicit Renderer2D(const std::shared_ptr<RenderContext>& context);

    void Begin();

    void PrepareBatch(sge::Batch& batch);
    void RenderBatch(sge::Batch& batch);
    void UploadBatchData();

    inline std::unique_ptr<sge::Batch> CreateBatch(const sge::BatchDesc& desc = {}) {
        return std::make_unique<sge::Batch>(*this, desc);
    }

    void DestroyBatch(sge::Batch& batch);

    void DrawPath(const sge::Path& path, const sge::LinearRgba& color = sge::LinearRgba::white(), const sge::Transform& transform = sge::Transform());

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
    void UpdateBatchBuffers(sge::Batch& batch);
    void ApplyBatchDrawCommands(sge::Batch& batch);

    void InitVectorPipeline();

private:
    BatchData<SpriteInstance> m_sprite_batch_data;
    BatchData<GlyphInstance> m_glyph_batch_data;
    BatchData<NinePatchInstance> m_ninepatch_batch_data;
    BatchData<ShapeInstance> m_shape_batch_data;
    BatchData<LineInstance> m_line_batch_data;

    Ref<LLGL::Shader> m_sprite_vertex_shader;
    Ref<LLGL::Shader> m_glyph_vertex_shader;
    Ref<LLGL::Shader> m_ninepatch_vertex_shader;

    Ref<LLGL::Shader> m_sprite_default_fragment_shader;
    Ref<LLGL::Shader> m_glyph_default_fragment_shader;

    LLGL::VertexFormat m_vector_vertex_format;

    Ref<LLGL::Shader> m_vector_vertex_shader;
    Ref<LLGL::Shader> m_vector_fragment_shader;
    Ref<LLGL::PipelineLayout> m_vector_stencil_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_stencil_pipeline;
    Ref<LLGL::PipelineLayout> m_vector_cover_pipeline_layout;
    Ref<LLGL::PipelineState> m_vector_cover_pipeline;
    Ref<LLGL::Buffer> m_vector_vertex_buffer;
    Ref<LLGL::Buffer> m_vector_path_data_buffer;

    uint32_t m_batch_path_total_count = 0;

    uint32_t m_batch_instance_count = 0;

    bool m_vector_pipeline_initialized = false;
};

} // namespace sge

#endif // _SGE_RENDERER_2D_HPP_