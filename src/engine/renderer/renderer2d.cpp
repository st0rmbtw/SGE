#include <algorithm>

#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>

#include <SGE/assert.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/attributes.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/utils/hash.hpp>

#include "shaders.hpp"

namespace {

using namespace sge::internal;

inline constexpr uint32_t VECTOR_VERTEX_BUFFER_SIZE = 10000;

// Stable LSD radix sort on the 64-bit key. Passes where all keys share the byte are skipped, so
// typically only 3-4 of the 8 passes run (order's low bytes + state id).
void RadixSortStable(std::vector<SortItem>& a, std::vector<SortItem>& tmp) {
    const size_t n = a.size();
    tmp.resize(n);

    std::array<std::array<uint32_t, 256>, 8> hist{};
    for (const SortItem& it : a)
        for (int p = 0; p < 8; ++p) ++hist[p][(it.key >> (p * 8)) & 0xFF];

    SortItem* src = a.data();
    SortItem* dst = tmp.data();
    for (int p = 0; p < 8; ++p) {
        if (hist[p][(src[0].key >> (p * 8)) & 0xFF] == n) continue;       // single bucket: nothing to do
        std::array<uint32_t, 256> offs;
        uint32_t sum = 0;
        for (int i = 0; i < 256; ++i) {
            offs[i] = sum;
            sum += hist[p][i];
        }
        for (size_t i = 0; i < n; ++i)
            dst[offs[(src[i].key >> (p * 8)) & 0xFF]++] = src[i];
        std::swap(src, dst);
    }
    if (src != a.data())
        std::memcpy(a.data(), src, n * sizeof(SortItem));
}

} // namespace

sge::Renderer2D::Renderer2D(const std::shared_ptr<RenderContext>& context) : Renderer(context) {
}

void sge::Renderer2D::SubmitBatch(std::shared_ptr<IBatch> batch) {
    ZoneScoped;

    auto keys = batch->GetSortKeys();
    if (keys.empty()) return;

    const auto& material = batch->GetMaterial(sge::BlendMode::AlphaBlend);
    const size_t instanceStride = material->GetInstanceAttribs()[0].stride;
    const auto* instancesData = static_cast<const uint8_t*>(batch->GetInstanceData());

    BatchSubmission submission;
    submission.states = &batch->GetStates();
    submission.batch = std::move(batch);
    submission.instanceStride = instanceStride;

    if (std::ranges::is_sorted(keys)) {
        // Painter-ordered input is already sorted: zero sorting, zero copying.
        submission.keysSpan = keys;
        submission.instancesBytes = instancesData;
    } else {
        const size_t n = keys.size();
        m_sort_items.resize(n);
        for (size_t i = 0; i < n; ++i)
            m_sort_items[i] = { .key=keys[i], .index=static_cast<uint32_t>(i) };
        RadixSortStable(m_sort_items, m_sort_tmp);

        if (m_submissions.size() >= m_scratch_pool.size()) {
            m_scratch_pool.resize(m_submissions.size() + 1);
        }
        SortScratch& scratch = m_scratch_pool[m_submissions.size()];
        scratch.keys.resize(n);
        scratch.instances.resize(n * instanceStride);

        // Gather: sequential writes
        for (size_t k = 0; k < n; ++k) {
            scratch.keys[k] = m_sort_items[k].key;
            std::memcpy(scratch.instances.data() + k * instanceStride, instancesData + size_t(m_sort_items[k].index) * instanceStride, instanceStride);
        }
        submission.keysSpan = scratch.keys;
        submission.instancesBytes = scratch.instances.data();
    }

    m_submissions.push_back(std::move(submission));
}

void sge::Renderer2D::FlushBatches() {
    ZoneScoped;

    while (true) {
        // Get Submission whose next draw has the lowest order
        BatchSubmission* cur = nullptr;
        uint32_t curOrder = 0;
        for (auto& s : m_submissions) {
            if (s.cursor >= s.keysSpan.size()) continue;
            const uint32_t o = internal::SortKeyOrder(s.keysSpan[s.cursor]);
            if (!cur || o < curOrder) {
                cur = &s;
                curOrder = o;
            }
        }
        if (!cur) break;

        // Find how far may we run before another batch must draw
        uint32_t limit = UINT32_MAX;
        for (const auto& s : m_submissions)
            if (&s != cur && s.cursor < s.keysSpan.size())
                limit = std::min(limit, internal::SortKeyOrder(s.keysSpan[s.cursor]));

        // Extend the run while the state is unchanged and the order stays within the limit
        const uint32_t stateId = internal::SortKeyState(cur->keysSpan[cur->cursor]);
        size_t end = cur->cursor + 1;
        while (
            end < cur->keysSpan.size()
                && internal::SortKeyState(cur->keysSpan[end]) == stateId
                && internal::SortKeyOrder(cur->keysSpan[end]) <= limit
        ) {
            ++end;
        }

        const internal::DrawCommandState& st = cur->states->Get(stateId);
        SubmitManyRaw(
            cur->batch->GetMesh(),
            cur->batch->GetMaterial(st.blendMode),
            st.Resources(),
            st.scissor,
            cur->instancesBytes + cur->cursor * cur->instanceStride,
            cur->instanceStride,
            end - cur->cursor
        );
        cur->cursor = end;
    }

    for (const BatchSubmission& s : m_submissions) {
        s.batch->Reset();
    }
    m_submissions.clear();
}

void sge::Renderer2D::InitVectorPipeline() {
    LLGL::VertexFormat vertexFormat = sge::VertexAttributes(m_context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_position", "Position")
    });
    m_vector_vertex_buffer = m_context->CreateVertexBuffer(VECTOR_VERTEX_BUFFER_SIZE * sizeof(glm::vec2), vertexFormat.GetStride(), "Vector Vertex Buffer");

    ShaderSourceCode shader = GetVectorShaderSourceCode(m_context->Backend());
    m_vector_vertex_shader = m_context->GetOrCreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size);
    m_vector_fragment_shader = m_context->GetOrCreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);

    {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = sge::BindingLayout({
            sge::BindingLayoutItem::ConstantBuffer(2, "UniformBuffer", LLGL::StageFlags::VertexStage),
            sge::BindingLayoutItem::ConstantBuffer(3, "PathDataBuffer", LLGL::StageFlags::VertexStage),
        });

        m_vector_stencil_pipeline_layout = m_context->CreatePipelineLayout(layoutDesc);

        LLGL::GraphicsPipelineDescriptor stencilPipelineDesc;
        stencilPipelineDesc.debugName = "Vector Stencil Pipeline";
        stencilPipelineDesc.pipelineLayout = m_vector_stencil_pipeline_layout;
        stencilPipelineDesc.vertexShader = m_vector_vertex_shader;
        stencilPipelineDesc.inputVertexAttribs = vertexFormat.attributes;

        stencilPipelineDesc.blend.targets[0].blendEnabled = false;
        stencilPipelineDesc.blend.targets[0].colorMask = LLGL::ColorMaskFlags::Zero;

        stencilPipelineDesc.stencil.testEnabled = true;

        stencilPipelineDesc.stencil.front.compareOp = LLGL::CompareOp::AlwaysPass;
        stencilPipelineDesc.stencil.front.stencilFailOp = LLGL::StencilOp::Keep;
        stencilPipelineDesc.stencil.front.depthFailOp = LLGL::StencilOp::Keep;
        stencilPipelineDesc.stencil.front.depthPassOp = LLGL::StencilOp::IncWrap;
        stencilPipelineDesc.stencil.front.writeMask = 0xFF;
        stencilPipelineDesc.stencil.front.readMask  = 0xFF;

        stencilPipelineDesc.stencil.back.compareOp = LLGL::CompareOp::AlwaysPass;
        stencilPipelineDesc.stencil.back.stencilFailOp = LLGL::StencilOp::Keep;
        stencilPipelineDesc.stencil.back.depthFailOp = LLGL::StencilOp::Keep;
        stencilPipelineDesc.stencil.back.depthPassOp = LLGL::StencilOp::DecWrap;
        stencilPipelineDesc.stencil.back.writeMask = 0xFF;
        stencilPipelineDesc.stencil.back.readMask  = 0xFF;

        m_vector_stencil_pipeline = m_context->CreatePipelineState(stencilPipelineDesc);
    }
    {
        LLGL::PipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindings = sge::BindingLayout({
            sge::BindingLayoutItem::ConstantBuffer(3, "PathDataBuffer", LLGL::StageFlags::FragmentStage)
        });
        m_vector_cover_pipeline_layout = m_context->CreatePipelineLayout(layoutDesc);

        LLGL::GraphicsPipelineDescriptor coverPipelineDesc;
        coverPipelineDesc.debugName = "Vector Cover Pipeline";
        coverPipelineDesc.pipelineLayout = m_vector_cover_pipeline_layout;
        coverPipelineDesc.vertexShader = m_vector_vertex_shader;
        coverPipelineDesc.fragmentShader = m_vector_fragment_shader;
        coverPipelineDesc.inputVertexAttribs = vertexFormat.attributes;

        coverPipelineDesc.stencil.testEnabled = true;
        coverPipelineDesc.stencil.front.compareOp = LLGL::CompareOp::NotEqual;
        coverPipelineDesc.stencil.front.stencilFailOp = LLGL::StencilOp::Zero;
        coverPipelineDesc.stencil.front.depthFailOp = LLGL::StencilOp::Keep;
        coverPipelineDesc.stencil.front.depthPassOp = LLGL::StencilOp::Zero;
        coverPipelineDesc.stencil.front.reference  = 0;
        coverPipelineDesc.stencil.front.readMask  = 0xFF;
        coverPipelineDesc.stencil.front.writeMask = 0xFF;
        coverPipelineDesc.stencil.back = coverPipelineDesc.stencil.front;

        coverPipelineDesc.blend = LLGL::BlendDescriptor {
            .targets = {
                LLGL::BlendTargetDescriptor {
                    .blendEnabled = true,
                    .srcColor = LLGL::BlendOp::SrcAlpha,
                    .dstColor = LLGL::BlendOp::InvSrcAlpha,
                    .srcAlpha = LLGL::BlendOp::Zero,
                    .dstAlpha = LLGL::BlendOp::One,
                    .alphaArithmetic = LLGL::BlendArithmetic::Max
                }
            }
        };

        m_vector_cover_pipeline = m_context->CreatePipelineState(coverPipelineDesc);
    }

    m_vector_path_data_buffer = m_context->CreateConstantBuffer(sizeof(PathData));

    m_vector_pipeline_initialized = true;
}

void sge::Renderer2D::DrawPath(const sge::Path& path, const sge::LinearRgba& color, const sge::Transform& transform) {
    if (!m_vector_pipeline_initialized)
        InitVectorPipeline();

    PathData pathData = {
        .transformMatrix = transform.ComputeMatrix(),
        .color = color.to_vec4()
    };

    m_command_buffer->UpdateBuffer(*m_vector_path_data_buffer, 0, &pathData, sizeof(PathData));

    const auto& vertices = path.GetTriangleVertices();
    SGE_ASSERT(vertices.size() < VECTOR_VERTEX_BUFFER_SIZE);
    UpdateBufferChunked(*m_command_buffer, *m_vector_vertex_buffer, 0, vertices.data(), vertices.size() * sizeof(glm::vec2));

    m_command_buffer->SetPipelineState(*m_vector_stencil_pipeline);
    m_command_buffer->SetVertexBuffer(*m_vector_vertex_buffer);
    m_command_buffer->SetResource(0, *GlobalUniformBuffer());
    m_command_buffer->SetResource(1, *m_vector_path_data_buffer);
    m_command_buffer->Draw(vertices.size(), 0);

    const auto bounds = path.GetBounds();

    float quad[] = {
        bounds.min.x, bounds.min.y,
        bounds.max.x, bounds.min.y,
        bounds.max.x, bounds.max.y,
        bounds.min.x, bounds.min.y,
        bounds.max.x, bounds.max.y,
        bounds.min.x, bounds.max.y,
    };
    m_command_buffer->UpdateBuffer(*m_vector_vertex_buffer, 0, &quad, sizeof(quad));

    m_command_buffer->SetPipelineState(*m_vector_cover_pipeline);
    m_command_buffer->SetVertexBuffer(*m_vector_vertex_buffer);
    m_command_buffer->SetResource(0, *m_vector_path_data_buffer);
    m_command_buffer->Draw(6, 0);
}
