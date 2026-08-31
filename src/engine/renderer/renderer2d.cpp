#include <ranges>

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

inline constexpr uint32_t VECTOR_VERTEX_BUFFER_SIZE = 10000;

bool DrawCommandStateEqual(const sge::internal::DrawCommandState& a, const sge::internal::DrawCommandState& b, LLGL::Resource* const* dynamicResources) {
    size_t hash_a = sge::HASH_INIT;
    sge::hash_fnv1a(hash_a, dynamicResources + a.resources_offset, a.resources_count);

    size_t hash_b = sge::HASH_INIT;
    sge::hash_fnv1a(hash_b, dynamicResources + b.resources_offset, b.resources_count);

    return hash_a == hash_b
        && a.scissor == b.scissor
        && a.order == b.order
        && a.blend_mode == b.blend_mode;
}


} // namespace

sge::Renderer2D::Renderer2D(const std::shared_ptr<RenderContext>& context) : Renderer(context) {
}

void sge::Renderer2D::SubmitBatch(std::shared_ptr<IBatch> batch) {
    ZoneScoped;

    auto& drawCommands = batch->GetDrawCommands();
    auto& material = batch->GetMaterial(sge::BlendMode::AlphaBlend);
    auto* instanceData = batch->GetInstanceData();
    auto* dynamicBindings = batch->GetDynamicBindings();

    if (drawCommands.empty())
        return;

    m_sort_indices.clear();
    m_sort_indices.reserve(drawCommands.size());
    std::ranges::copy(std::views::iota(0u, drawCommands.size()), std::back_inserter(m_sort_indices));

    std::ranges::sort(m_sort_indices, [&](const size_t ia, const size_t ib) {
        const DrawCommand& a = drawCommands[ia];
        const DrawCommand& b = drawCommands[ib];

        if (a.state.order < b.state.order) return true;
        if (a.state.order > b.state.order) return false;

        const int a_scissor_size = a.state.scissor.width() + a.state.scissor.height();
        const int b_scissor_size = b.state.scissor.width() + b.state.scissor.height();

        if (a_scissor_size < b_scissor_size)
            return true;

        if (a_scissor_size > b_scissor_size)
            return false;

        uint8_t a_bm = static_cast<uint8_t>(a.state.blend_mode);
        uint8_t b_bm = static_cast<uint8_t>(b.state.blend_mode);

        if (a_bm < b_bm) return true;
        if (a_bm > b_bm) return false;

        size_t hash_a = sge::HASH_INIT;
        sge::hash_fnv1a(hash_a, dynamicBindings + a.state.resources_offset, a.state.resources_count);

        size_t hash_b = sge::HASH_INIT;
        sge::hash_fnv1a(hash_b, dynamicBindings + b.state.resources_offset, b.state.resources_count);

        if (hash_a < hash_b) return true;
        if (hash_a > hash_b) return false;

        return false;
    });

    const auto& instanceAttribs = material->GetInstanceAttribs();
    SGE_ASSERT(!instanceAttribs.empty());

    auto* instancePtr = static_cast<uint8_t*>(instanceData);
    const auto instanceStride = instanceAttribs[0].stride;

    static constexpr size_t TEMP_BUFFER_SIZE = 256;
    SGE_ASSERT(instanceStride <= TEMP_BUFFER_SIZE);
    uint8_t tempBuf[TEMP_BUFFER_SIZE];

    // Sort instances and draw commands
    for (size_t i = 0; i < m_sort_indices.size(); ++i) {
        size_t currentIdx = i;

        while (m_sort_indices[currentIdx] != i) {
            size_t nextIdx = m_sort_indices[currentIdx];

            std::swap(drawCommands[currentIdx], drawCommands[nextIdx]);

            auto* elemCurr = instancePtr + (currentIdx * instanceStride);
            auto* elemNext = instancePtr + (nextIdx * instanceStride);

            std::memcpy(tempBuf, elemCurr, instanceStride);
            std::memcpy(elemCurr, elemNext, instanceStride);
            std::memcpy(elemNext, tempBuf, instanceStride);

            m_sort_indices[currentIdx] = currentIdx;
            currentIdx = nextIdx;
        }
        m_sort_indices[currentIdx] = currentIdx;
    }

    m_submissions.push_back(BatchSubmission {
        .batch = std::move(batch),
        .instancesData = instancePtr,
        .instanceStride = instanceStride,
        .dynamicBindings = dynamicBindings,
        .drawCommands = drawCommands,
        .drawCommandsOffset = 0,
    });
}

void sge::Renderer2D::FlushBatches() {
    ZoneScoped;

    auto GetNextOrder = [&]() -> std::optional<uint32_t> {
        std::optional<uint32_t> min;
        auto consider = [&](uint32_t order) {
            if (!min || order < *min) min = order;
        };

        for (const auto& s : m_submissions) {
            if (s.drawCommandsOffset < s.drawCommands.size()) {
                consider(s.drawCommands[s.drawCommandsOffset].state.order);
            }
        }

        return min;
    };

    m_batch_data.clear();
    for (const auto& submission : m_submissions) {
        // Draw commands are sorted, so checking only first and last is enough
        const bool allTheSame = DrawCommandStateEqual(submission.drawCommands.front().state, submission.drawCommands.back().state, submission.dynamicBindings);
        m_batch_data.push_back(BatchData {
            .state = submission.drawCommands[0].state,
            .offset = 0,
            .allTheSame = allTheSame
        });
    }

    while (true) {
        auto currentOrder = GetNextOrder();
        if (!currentOrder)
            break;

        for (size_t submission_index = 0; submission_index < m_submissions.size(); ++submission_index) {
            auto& submission = m_submissions[submission_index];

            if (submission.drawCommandsOffset >= submission.drawCommands.size())
                continue;

            auto& data = m_batch_data[submission_index];

            const size_t instanceStride = submission.instanceStride;

            // All draw commands are the same - flush the whole batch right away
            if (data.allTheSame && currentOrder == submission.drawCommands[0].state.order) {
                submission.drawCommandsOffset = submission.drawCommands.size();
                auto dynamicBindings = std::span<LLGL::Resource* const>(submission.dynamicBindings, data.state.resources_count);
                const auto& mesh = submission.batch->GetMesh();
                const auto& material = submission.batch->GetMaterial(data.state.blend_mode);
                SubmitManyRaw(mesh, material, dynamicBindings, data.state.scissor, submission.instancesData, instanceStride, submission.drawCommands.size());
            } else {
                size_t i = submission.drawCommandsOffset;
                if (submission.drawCommands[i].state.order == currentOrder) {
                    data.state = submission.drawCommands[i].state;

                    for (; i < submission.drawCommands.size(); ++i) {
                        const DrawCommand& command = submission.drawCommands[i];

                        if (!DrawCommandStateEqual(command.state, data.state, submission.dynamicBindings))
                            break;
                    }
                }
                const uint32_t count = i - submission.drawCommandsOffset;
                submission.drawCommandsOffset = i;

                if (count > 0) {
                    auto dynamicBindings = std::span<LLGL::Resource* const>(submission.dynamicBindings + data.state.resources_offset, data.state.resources_count);
                    const auto* instanceDataBytes = static_cast<const uint8_t*>(submission.instancesData);
                    const auto& mesh = submission.batch->GetMesh();
                    const auto& material = submission.batch->GetMaterial(data.state.blend_mode);
                    SubmitManyRaw(mesh, material, dynamicBindings, data.state.scissor, instanceDataBytes + data.offset * instanceStride, instanceStride, count);
                    data.offset += count;
                }
            }
        }
    }

    for (const BatchSubmission& submission : m_submissions) {
        submission.batch->Reset();
    }

    m_submissions.clear();
}

void sge::Renderer2D::InitVectorPipeline() {
    LLGL::VertexFormat vertexFormat = sge::VertexAttributes(m_context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_position", "Position")
    });
    m_vector_vertex_buffer = m_context->CreateVertexBuffer(VECTOR_VERTEX_BUFFER_SIZE * sizeof(glm::vec2), vertexFormat.GetStride(), "Vector Vertex Buffer");

    ShaderSourceCode shader = GetVectorShaderSourceCode(m_context->Backend());
    m_vector_vertex_shader = m_context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size);
    m_vector_fragment_shader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);

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
