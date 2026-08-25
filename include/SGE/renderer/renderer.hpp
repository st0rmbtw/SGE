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
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/framebuffer_pool.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/utils/hash.hpp>

#include <memory>

namespace sge {

struct alignas(16) GlobalUniforms {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::vec2 camera_position;
    glm::uvec2 window_size;
};

class Renderer {
public:
    explicit Renderer(const std::shared_ptr<RenderContext>& context);

    inline void Begin() {
        m_command_buffer->Begin();
    }

    inline void End() {
        m_command_buffer->End();
        m_command_queue->Submit(*m_command_buffer);
        m_context->TickTemporaryFramebufferPool();
    }

    inline void BeginPass(LLGL::RenderTarget& target) {
        m_context->PushRenderTarget(&target);
        m_command_buffer->BeginRenderPass(target);
    }
    inline void BeginPass(const std::shared_ptr<GlfwWindow>& window) {
        BeginPass(m_context->GetOrCreateSwapChain(window));
    }

    void BeginPass(LLGL::RenderTarget& target, const Camera& camera);
    inline void BeginPass(const std::shared_ptr<GlfwWindow>& window, const Camera& camera) {
        BeginPass(m_context->GetOrCreateSwapChain(window), camera);
    }

    void TonemapPass(sge::Framebuffer& framebuffer);
    void BloomPass(sge::Framebuffer& framebuffer, const sge::BloomSettings& settings = {});

    inline void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
    }

    void EndPass();

    void BlitTexture(LLGL::Texture& texture);

    inline void SetScissor(const LLGL::Scissor& scissor) {
        m_command_buffer->SetScissor(scissor);
    }

    void Present(const std::shared_ptr<sge::GlfwWindow>& window) {
        m_context->Present(*window);
    }

    template <typename TInstance>
    void Submit(Ref<Mesh> mesh, Ref<Material> material, const TInstance& instance) {
        SubmitRaw(mesh, material, &instance, sizeof(TInstance));
    }

    void Submit(Ref<Mesh> mesh, Ref<Material> material, std::span<LLGL::Resource* const> dynamicResources = {}) {
        SubmitRaw(std::move(mesh), std::move(material), dynamicResources, nullptr, 0);
    }

    void SubmitRaw(
        Ref<Mesh> mesh,
        Ref<Material> material,
        std::span<LLGL::Resource* const> dynamicResources,
        const void* instanceData,
        size_t instanceByteSize
    );

    void FlushManyRaw(
        Ref<Mesh> mesh,
        Ref<Material> material,
        std::span<LLGL::Resource* const> dynamicResources,
        sge::IRect scissorBounds,
        const void* instanceData,
        size_t instanceDataSize,
        uint32_t instanceCount
    );

    Ref<Mesh> CreateMesh(const sge::MeshDesc& desc) {
        return Ref<Mesh>::Create(*m_context, desc);
    }

    Ref<Material> CreateMaterial(const sge::MaterialDesc& desc) {
        return Ref<Material>::Create(*m_context, desc);
    }

    LLGL::PipelineState& GetOrCreatePipeline(
        const Material& material,
        const LLGL::VertexFormat& vertexFormat,
        sge::PrimitiveTopology topology,
        sge::FrontFace frontFace,
        sge::IndexFormat indexFormat
    );

    sge::Ref<sge::Mesh> GetDefaultBatch2dMesh();

    [[nodiscard]]
    inline LLGL::CommandBuffer* CommandBuffer() const noexcept {
        return m_command_buffer;
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* CommandQueue() const noexcept {
        return m_command_queue;
    }

    [[nodiscard]]
    inline const Ref<LLGL::Buffer>& GlobalUniformBuffer() const noexcept {
        return m_uniform_buffer;
    }

    [[nodiscard]]
    inline const std::shared_ptr<RenderContext>& GetRenderContext() const noexcept {
        return m_context;
    }

    [[nodiscard]]
    inline const Ref<LLGL::Buffer>& FullscreenTriangleVertexBuffer() const noexcept {
        return m_fullscreen_triangle_vertex_buffer;
    }

    [[nodiscard]]
    inline const LLGL::VertexFormat& FullscreenTriangleVertexFormat() const noexcept {
        return m_fullscreen_triangle_vertex_format;
    }

    [[nodiscard]]
    inline const Ref<LLGL::Shader>& FullscreenTriangleVertexShader() const noexcept {
        return m_fullscreen_triangle_vertex_shader;
    }

    [[nodiscard]]
    inline const Ref<LLGL::Shader>& BlitShader() const noexcept {
        return m_blit_pixel_shader;
    }

    [[nodiscard]]
    inline const Ref<LLGL::PipelineLayout>& BlitPipelineLayout() const noexcept {
        return m_blit_pipeline_layout;
    }

    [[nodiscard]]
    inline const Ref<LLGL::PipelineState>& BlitPipeline() const noexcept {
        return m_blit_pipeline;
    }

private:
    void InitBloomPipelines();
    void InitTonemapPipelines();

    void FlushBatchRawImpl(
        const Mesh& mesh,
        const Material& material,
        sge::IRect scissorBounds, // If the material has scissor test disabled - this parameter ignored
        std::span<LLGL::Resource* const> dynamicBindings,
        sge::Unique<LLGL::BufferArray>& bufferArray,
        VertexBufferPool& instanceBuffer,
        const void* instanceData,
        size_t instanceStride,
        uint32_t instanceCount
    );

protected:
    struct PipelineKey {
        uint64_t vertexFormatHash = 0;
        uint64_t materialHash = 0;

        friend bool operator==(const PipelineKey& a, const PipelineKey& b) noexcept {
            return a.vertexFormatHash == b.vertexFormatHash && a.materialHash == b.materialHash;
        }
    };

    struct PipelineKeyHasher {
        size_t operator()(const PipelineKey& key) const noexcept {
            size_t hash = 0;
            hash_combine(hash, key.vertexFormatHash);
            hash_combine(hash, key.materialHash);
            return hash;
        }
    };

    struct BatchKey {
        Mesh* mesh;
        Material* material;
        uint64_t dynamicBindingsHash = 0;
        uint64_t scissorHash = 0;

        friend bool operator==(const BatchKey& a, const BatchKey& b) noexcept {
            return a.mesh == b.mesh
                && a.material == b.material
                && a.dynamicBindingsHash == b.dynamicBindingsHash
                && a.scissorHash == b.scissorHash;
        }
    };

    struct BatchKeyHasher {
        size_t operator()(const BatchKey& key) const noexcept {
            size_t hash = 0;
            hash_combine(hash, key.mesh);
            hash_combine(hash, key.material);
            hash_combine(hash, key.dynamicBindingsHash);
            hash_combine(hash, key.scissorHash);
            return hash;
        }
    };

    struct MeshBatch {
        sge::VertexBufferPool instanceBufferPool;
        sge::Unique<LLGL::BufferArray> vertexBufferArray;
        std::vector<uint8_t> instanceBytes;
        std::vector<LLGL::Resource*> dynamicBindings;
        sge::Ref<Mesh> mesh;
        sge::Ref<Material> material;
        sge::IRect scissorBounds;
        size_t instanceCount;
    };

    std::unordered_map<PipelineKey, PipelineId, PipelineKeyHasher> m_pipelines;
    std::unordered_map<BatchKey, size_t, BatchKeyHasher> m_batches_map;
    std::vector<MeshBatch> m_mesh_batches;

    LLGL::VertexFormat m_fullscreen_triangle_vertex_format;

    std::vector<sge::TemporaryFramebuffer> m_bloom_framebuffers;
    Unique<LLGL::Buffer> m_bloom_cb;
    Unique<LLGL::RenderPass> m_bloom_render_pass;
    Unique<LLGL::PipelineState> m_bloom_prefilter_pipeline;
    Unique<LLGL::PipelineState> m_bloom_downsample_pipeline;
    Unique<LLGL::PipelineState> m_bloom_upsample_pipeline;
    Unique<LLGL::PipelineState> m_bloom_composite_pipeline;

    Unique<LLGL::RenderPass> m_tonemap_render_pass;
    Unique<LLGL::PipelineState> m_tonemap_aces_pipeline;

    Ref<LLGL::Buffer> m_fullscreen_triangle_vertex_buffer;
    Ref<LLGL::Shader> m_fullscreen_triangle_vertex_shader;

    Ref<LLGL::Shader> m_blit_pixel_shader;
    Ref<LLGL::PipelineLayout> m_blit_pipeline_layout;
    Ref<LLGL::PipelineState> m_blit_pipeline;
    Ref<LLGL::RenderPass> m_blit_render_pass;

    Ref<LLGL::Buffer> m_uniform_buffer;

    Ref<sge::Mesh> m_2d_batch_mesh;

    std::shared_ptr<RenderContext> m_context;

    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;

    LLGL::Extent2D m_viewport = LLGL::Extent2D(0, 0);

    BloomSettings m_prev_bloom_settings = { .maxIterations = 0 };
};

} // namespace sge

#endif
