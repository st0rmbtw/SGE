#if __has_include(<execution>)
#include <execution>
#endif

#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>

#include <SGE/profile.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/binding_layout.hpp>

#include "SGE/assert.hpp"
#include "shaders.hpp"

// GNU PSTL doesn't compile with exceptions disabled
#if (!defined(__GNUC__) || defined(__cpp_exceptions)) && defined(__cpp_lib_execution)
    #define PARALLEL 1
#else
    #define PARALLEL 0
#endif

static constexpr uint32_t DEFAULT_BATCH_COUNT = 2000;
static constexpr uint32_t VECTOR_VERTEX_BUFFER_SIZE = 10000;

namespace {

namespace SpriteFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace GlyphFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace NinePatchFlags {
    enum : uint8_t {
        UI = 0,
    };
};

namespace ShapeFlags {
    enum : uint8_t {
        UI = 0,
    };
};

struct BatchVertexFormats {
    LLGL::VertexFormat vertex;
    LLGL::VertexFormat instance;

    [[nodiscard]]
    std::vector<LLGL::VertexAttribute> total_attributes() const {
        std::vector<LLGL::VertexAttribute> attributes = vertex.attributes;
        attributes.insert(attributes.end(), instance.attributes.begin(), instance.attributes.end());
        return attributes;
    }
};

BatchVertexFormats SpriteBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_outline_color", "I_OutlineColor", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_outline_thickness", "I_OutlineThickness", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats NinepatchBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });

    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint32x4,  "inp_i_margin", "I_Margin", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_source_size", "I_SourceSize", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_output_size", "I_OutputSize", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    return {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats GlyphVectorBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_em_size", "I_Em_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint32, "inp_i_partition_offset", "I_PartitionOffset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint32, "inp_i_partition_count", "I_PartitionCount", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats GlyphSDFBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_tex_size", "I_TexSize", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_uv", "I_UV", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats ShapeBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });

    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_color", "I_BorderColor", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_BorderRadius", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_border_thickness", "I_BorderThickness", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_shape", "I_Shape", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1)
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats LineBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = sge::VertexAttributes(backend, {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format;
    instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_start", "I_Start", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_end", "I_End", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_Border_Radius", 1),
        sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_thickness", "I_Thickness", 1),
        sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

SGE_FORCE_INLINE sge::Ref<LLGL::Shader> CreateBatchVertexShader(const std::shared_ptr<sge::RenderContext>& context, const ShaderSourceCode& source_code, const BatchVertexFormats& vertex_formats) {
    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();
    return context->CreateShader(sge::ShaderType::Vertex, "VS", source_code.vs_source, source_code.vs_size, shaderConfig);
}

SGE_FORCE_INLINE sge::Handle<LLGL::PipelineState> GetPipelineByBlendMode(sge::BlendMode blend_mode, const sge::SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case sge::BlendMode::AlphaBlend: return pipeline.alpha_blend;
    case sge::BlendMode::Additive: return pipeline.additive;
    case sge::BlendMode::Opaque: return pipeline.opaque;
    case sge::BlendMode::PremultipliedAlpha: return pipeline.premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

SGE_FORCE_INLINE sge::Handle<LLGL::PipelineState> GetDepthPipelineByBlendMode(sge::BlendMode blend_mode, const sge::SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case sge::BlendMode::AlphaBlend: return pipeline.depth_alpha_blend;
    case sge::BlendMode::Additive: return pipeline.depth_additive;
    case sge::BlendMode::Opaque: return pipeline.depth_opaque;
    case sge::BlendMode::PremultipliedAlpha: return pipeline.depth_premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

} // namespace

template <typename T>
void sge::BatchData<T>::Init(sge::RenderContext& context, uint32_t count, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format) {
    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    m_vertex_buffer = context.CreateVertexBuffer(vertices, vertex_format, "Batch VertexBuffer");
    m_instance_format = instance_format;
    
    CreateDynamicBuffers(context, count);
}

template <typename T>
void sge::BatchData<T>::CreateDynamicBuffers(sge::RenderContext& context, uint32_t count) {
    if (m_instance_buffer)
        context.Release(*m_instance_buffer);
    if (m_buffer_array)
        context.Release(*m_buffer_array);

    m_buffer = sge::HeapArray<T>(count);
    m_buffer_ptr = m_buffer.data();
    
    m_instance_buffer = context.CreateVertexBuffer(count * sizeof(T), m_instance_format, "Batch InstanceBuffer");
    m_buffer_array = context.CreateBufferArray({ m_vertex_buffer.Get(), m_instance_buffer.Get() });
    m_max_count = count;
}

sge::Renderer2D::Renderer2D(const std::shared_ptr<RenderContext>& context) : Renderer(context) {
    const RenderBackend backend = context->Backend();

    m_sprite_vertex_shader = CreateBatchVertexShader(context, GetSpriteShaderSourceCode(backend), SpriteBatchVertexFormats(backend));
    m_glyph_vector_vertex_shader = CreateBatchVertexShader(context, GetFontVectorShaderSourceCode(backend), GlyphVectorBatchVertexFormats(backend));
    m_glyph_sdf_vertex_shader = CreateBatchVertexShader(context, GetFontSdfShaderSourceCode(backend), GlyphSDFBatchVertexFormats(backend));
    m_ninepatch_vertex_shader = CreateBatchVertexShader(context, GetNinepatchShaderSourceCode(backend), NinepatchBatchVertexFormats(backend));

    m_sprite_batch_data = InitSpriteBatchData();
    m_ninepatch_batch_data = InitNinepatchBatchData();
    m_glyph_vector_batch_data = InitVectorGlyphBatchData();
    m_glyph_sdf_batch_data = InitSDFGlyphBatchData();
    m_shape_batch_data = InitShapeBatchData();
    m_line_batch_data = InitLineBatchData();


    {
        ShaderSourceCode shader = GetSpriteShaderSourceCode(backend);
        m_sprite_default_fragment_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }

    {
        ShaderSourceCode shader = GetFontVectorShaderSourceCode(backend);
        m_glyph_vector_fragment_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }

    {
        ShaderSourceCode shader = GetFontSdfShaderSourceCode(backend);
        m_glyph_sdf_default_fragment_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }

    m_vector_vertex_format.attributes = sge::VertexAttributes(m_context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_position", "Position")
    });
    m_vector_vertex_buffer = m_context->CreateVertexBuffer(VECTOR_VERTEX_BUFFER_SIZE * sizeof(glm::vec2), m_vector_vertex_format, "Vector Vertex Buffer");
}

void sge::Renderer2D::Begin() {
    m_batch_instance_count = 0;

    m_sprite_batch_data.Reset();
    m_glyph_vector_batch_data.Reset();
    m_glyph_sdf_batch_data.Reset();
    m_ninepatch_batch_data.Reset();
    m_shape_batch_data.Reset();
    m_line_batch_data.Reset();

    Renderer::Begin();
}

sge::SpriteBatchPipeline sge::Renderer2D::CreateSpriteBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader) {
    if (!fragment_shader) {
        fragment_shader = m_sprite_default_fragment_shader;
    }

    static uint32_t count = 0;

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.vertexShader = m_sprite_vertex_shader;
    pipelineConfig.pixelShader = fragment_shader;
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;

    LLGL::BlendDescriptor blend_modes[4] = {
        // AlphaBlend
        LLGL::BlendDescriptor {
            .targets = {
                LLGL::BlendTargetDescriptor {
                    .blendEnabled = true,
                    .srcColor = LLGL::BlendOp::SrcAlpha,
                    .dstColor = LLGL::BlendOp::InvSrcAlpha,
                    .srcAlpha = LLGL::BlendOp::SrcAlpha,
                    .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
                }
            }
        },
        // Additive
        LLGL::BlendDescriptor {
            .targets = {
                LLGL::BlendTargetDescriptor {
                    .blendEnabled = true,
                    .srcColor = LLGL::BlendOp::SrcAlpha,
                    .dstColor = LLGL::BlendOp::One,
                    .srcAlpha = LLGL::BlendOp::SrcAlpha,
                    .dstAlpha = LLGL::BlendOp::One,
                }
            }
        },
        // Opaque
        LLGL::BlendDescriptor {
            .targets = {
                LLGL::BlendTargetDescriptor {
                    .blendEnabled = true,
                    .srcColor = LLGL::BlendOp::One,
                    .dstColor = LLGL::BlendOp::Zero,
                    .srcAlpha = LLGL::BlendOp::One,
                    .dstAlpha = LLGL::BlendOp::Zero,
                }
            }
        },
        // PremultipliedAlpha
        LLGL::BlendDescriptor {
            .targets = {
                LLGL::BlendTargetDescriptor {
                    .blendEnabled = true,
                    .srcColor = LLGL::BlendOp::One,
                    .dstColor = LLGL::BlendOp::InvSrcAlpha,
                    .srcAlpha = LLGL::BlendOp::One,
                    .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
                }
            }
        }
    };

    SpriteBatchPipeline pipeline;

    {
        std::tuple<sge::BlendMode, sge::Handle<LLGL::PipelineState>*, const char*, int> pipelines[4] = {
            {
                sge::BlendMode::AlphaBlend, &pipeline.alpha_blend, "SpriteBatchPipelineAlphaBlend", 0
            },
            {
                sge::BlendMode::Additive, &pipeline.additive, "SpriteBatchPipelineAdditive", 1
            },
            {
                sge::BlendMode::Opaque, &pipeline.opaque, "SpriteBatchPipelineOpaque", 2
            },
            {
                sge::BlendMode::PremultipliedAlpha, &pipeline.premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlpha", 3
            },
        };

        for (const auto& [blend_mode, pointer, pipelineName, index] : pipelines) {
            pipelineConfig.debugName = std::format("{}_{}", pipelineName, count);
            
            // bool hasInitialCache = false;
            // auto pipelineCache = ReadPipelineCache(name, hasInitialCache);

            pipelineConfig.blend = blend_modes[index];
            // LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache.get());
            *pointer = GetRenderContext()->CreatePipelineState(pipelineConfig);

            // if (!hasInitialCache) {
            //     SavePipelineCache(name, *pipelineCache);
            // }

            // if (const LLGL::Report* report = pipeline->GetReport()) {
            //     if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            // }
        }
    }
    {
        std::tuple<sge::BlendMode, sge::Handle<LLGL::PipelineState>*, const char*, int> pipelines[4] = {
            {
                sge::BlendMode::AlphaBlend, &pipeline.depth_alpha_blend, "SpriteBatchPipelineAlphaBlendDepth", 0
            },
            {
                sge::BlendMode::Additive, &pipeline.depth_additive, "SpriteBatchPipelineAdditiveDepth", 1
            },
            {
                sge::BlendMode::Opaque, &pipeline.depth_opaque, "SpriteBatchPipelineOpaqueDepth", 2
            },
            {
                sge::BlendMode::PremultipliedAlpha, &pipeline.depth_premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlphaDepth", 3
            },
        };

        GraphicsPipelineConfig depthPipelineConfig = pipelineConfig;
        depthPipelineConfig.depth = LLGL::DepthDescriptor {
            .testEnabled = true,
            .writeEnabled = true,
            .compareOp = LLGL::CompareOp::GreaterEqual,
        };

        for (const auto& [blend_mode, pointer, pipelineName, index] : pipelines) {
            depthPipelineConfig.debugName = pipelineName;
            *pointer = GetRenderContext()->CreatePipelineState(depthPipelineConfig);
        }
    }

    ++count;

    return pipeline;
}

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateNinepatchBatchPipeline(bool enable_scissor) {
    const RenderBackend backend = GetRenderContext()->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    ShaderSourceCode shader = GetNinepatchShaderSourceCode(backend);

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "NinePatchBatch Pipeline";
    pipelineConfig.vertexShader = m_ninepatch_vertex_shader;
    pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;
    pipelineConfig.blend = LLGL::BlendDescriptor {
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

    return GetRenderContext()->CreatePipelineState(pipelineConfig);
}

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateGlyphSDFBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader) {
    if (!fragment_shader) {
        fragment_shader = m_glyph_sdf_default_fragment_shader;
    }

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });
    pipelineLayoutDesc.combinedTextureSamplers = {
        LLGL::CombinedTextureSamplerDescriptor{ "Texture", "Texture", "Sampler", 3 }
    };

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "GlyphBatch Pipeline";
    pipelineConfig.vertexShader = m_glyph_sdf_vertex_shader;
    pipelineConfig.pixelShader = fragment_shader;
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;
    pipelineConfig.blend = LLGL::BlendDescriptor {
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

    return GetRenderContext()->CreatePipelineState(pipelineConfig);
}

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateGlyphVectorBatchPipeline(bool enable_scissor) {
    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Buffer(3, "CurveBuffer", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Buffer(4, "PartitionBuffer", LLGL::StageFlags::FragmentStage),
    });
    pipelineLayoutDesc.combinedTextureSamplers = {
        LLGL::CombinedTextureSamplerDescriptor{ "Texture", "Texture", "Sampler", 3 }
    };

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "GlyphBatch Pipeline";
    pipelineConfig.vertexShader = m_glyph_vector_vertex_shader;
    pipelineConfig.pixelShader = m_glyph_vector_fragment_shader;
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;
    pipelineConfig.blend = LLGL::BlendDescriptor {
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

    return GetRenderContext()->CreatePipelineState(pipelineConfig);
}

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateShapeBatchPipeline(bool enable_scissor) {
    const RenderBackend backend = GetRenderContext()->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    ShaderSourceCode shader = GetShapeShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(backend);

    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "ShapeBatch Pipeline";
    pipelineConfig.vertexShader = GetRenderContext()->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
    pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;
    pipelineConfig.blend = LLGL::BlendDescriptor {
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

    return GetRenderContext()->CreatePipelineState(pipelineConfig);
}

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateLineBatchPipeline(bool enable_scissor) {
    const RenderBackend backend = GetRenderContext()->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    ShaderSourceCode shader = GetLineShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = LineBatchVertexFormats(backend);
    
    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "LineBatch Pipeline";
    pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
    pipelineConfig.vertexShader = GetRenderContext()->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
    pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.scissorTestEnabled = enable_scissor;
    pipelineConfig.blend = LLGL::BlendDescriptor {
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

    return GetRenderContext()->CreatePipelineState(pipelineConfig);
}


sge::BatchData<sge::SpriteInstance> sge::Renderer2D::InitSpriteBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = SpriteBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<SpriteInstance> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

sge::BatchData<sge::NinePatchInstance> sge::Renderer2D::InitNinepatchBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = NinepatchBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<NinePatchInstance> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

sge::BatchData<sge::GlyphInstanceVector> sge::Renderer2D::InitVectorGlyphBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = GlyphVectorBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<GlyphInstanceVector> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

sge::BatchData<sge::GlyphInstanceSDF> sge::Renderer2D::InitSDFGlyphBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = GlyphSDFBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<GlyphInstanceSDF> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

sge::BatchData<sge::ShapeInstance> sge::Renderer2D::InitShapeBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<ShapeInstance> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

sge::BatchData<sge::LineInstance> sge::Renderer2D::InitLineBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = LineBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<LineInstance> batchData;
    batchData.Init(*GetRenderContext(), DEFAULT_BATCH_COUNT, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

void sge::Renderer2D::DestroyBatch(sge::Batch& batch) {
    const SpriteBatchPipeline& sprite_pipeline = batch.SpritePipeline();
    const auto glyph_vector_pipeline = batch.GlyphVectorPipeline();
    const auto glyph_sdf_pipeline = batch.GlyphSDFPipeline();
    const auto line_pipeline = batch.LinePipeline();
    const auto shape_pipeline = batch.ShapePipeline();

    auto& context = GetRenderContext();

    context->DeletePipeline(sprite_pipeline.additive);
    context->DeletePipeline(sprite_pipeline.alpha_blend);
    context->DeletePipeline(sprite_pipeline.premultiplied_alpha);
    context->DeletePipeline(sprite_pipeline.opaque);
    context->DeletePipeline(sprite_pipeline.depth_additive);
    context->DeletePipeline(sprite_pipeline.depth_alpha_blend);
    context->DeletePipeline(sprite_pipeline.depth_premultiplied_alpha);
    context->DeletePipeline(sprite_pipeline.depth_opaque);
    context->DeletePipeline(glyph_vector_pipeline);
    context->DeletePipeline(glyph_sdf_pipeline);
    context->DeletePipeline(line_pipeline);
    context->DeletePipeline(shape_pipeline);
}

void sge::Renderer2D::ApplyBatchDrawCommands(sge::Batch& batch) {
    ZoneScoped;

    using namespace sge::internal;

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    if (flush_queue.empty())
        return;

    auto& context = GetRenderContext();
    auto* const commands = CommandBuffer();

    int prev_flush_data_type = -1;
    int prev_texture_id = -1;

    sge::BlendMode prev_blend_mode = flush_queue[0].blend_mode;
    
    Handle<LLGL::PipelineState> sprite_pipeline = batch.DepthEnabled()
        ? GetDepthPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline())
        : GetPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline());

    size_t offset = 0;

    for (const FlushData& flush_data : flush_queue) {
        if (prev_blend_mode != flush_data.blend_mode) {
            sprite_pipeline = batch.DepthEnabled()
                ? GetDepthPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline())
                : GetPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline());
        }

        if (prev_flush_data_type != static_cast<int>(flush_data.type) || prev_blend_mode != flush_data.blend_mode) {
            switch (flush_data.type) {
            case FlushDataType::Sprite:
                commands->SetVertexBufferArray(*m_sprite_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(sprite_pipeline));
                offset = batch.SpriteData().offset;
            break;

            case FlushDataType::GlyphVector:
                commands->SetVertexBufferArray(*m_glyph_vector_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.GlyphVectorPipeline()));
                offset = batch.GlyphVectorData().offset;
            break;

            case FlushDataType::GlyphSDF:
                commands->SetVertexBufferArray(*m_glyph_sdf_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.GlyphSDFPipeline()));
                offset = batch.GlyphSDFData().offset;
            break;

            case FlushDataType::NinePatch:
                commands->SetVertexBufferArray(*m_ninepatch_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.NinepatchPipeline()));
                offset = batch.NinepatchData().offset;
            break;

            case FlushDataType::Shape:
                commands->SetVertexBufferArray(*m_shape_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.ShapePipeline()));
                offset = batch.ShapeData().offset;
            break;
            case FlushDataType::Line:
                commands->SetVertexBufferArray(*m_line_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.LinePipeline()));
                offset = batch.LineData().offset;
            break;
            case internal::FlushDataType::COUNT:
                SGE_UNREACHABLE();
            break;
            }

            commands->SetResource(0, *GlobalUniformBuffer());
        }

        if (flush_data.scissor.width() > 0 && flush_data.scissor.height() > 0) {
            commands->SetScissor(LLGL::Scissor(
                flush_data.scissor.min.x,
                flush_data.scissor.min.y,
                flush_data.scissor.width(),
                flush_data.scissor.height()
            ));
        } else {
            commands->SetScissor(LLGL::Scissor(0, 0, m_viewport.width, m_viewport.height));
        }

        if (flush_data.type == FlushDataType::GlyphVector) {
            commands->SetResource(1, *flush_data.glyph_data.curve_buffer);
            commands->SetResource(2, *flush_data.glyph_data.partition_buffer);
            prev_texture_id = -1;
        } else if (flush_data.texture.is_valid() && prev_texture_id != flush_data.texture.id) {
            commands->SetResource(1, *flush_data.texture.ptr);
            commands->SetResource(2, *flush_data.texture.sampler);
            prev_texture_id = flush_data.texture.id;
        }

        commands->DrawInstanced(4, 0, flush_data.count, offset + flush_data.offset);

        prev_flush_data_type = static_cast<int>(flush_data.type);
        prev_blend_mode = flush_data.blend_mode;
    }

    flush_queue.clear();
}

void sge::Renderer2D::SortBatchDrawCommands(sge::Batch& batch) {
    ZoneScoped;

    using namespace sge::internal;

    auto& sprite_commands = batch.sprite_draw_commands();
    auto& glyph_vector_commands = batch.glyph_vector_draw_commands();
    auto& glyph_sdf_commands = batch.glyph_sdf_draw_commands();
    auto& ninepatch_commands = batch.ninepatch_draw_commands();
    auto& shape_commands = batch.shape_draw_commands();
    auto& line_commands = batch.line_draw_commands();

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        sprite_commands.begin(),
        sprite_commands.end(),
        [](const DrawCommandSprite& a, const DrawCommandSprite& b) {
            return SortTextureBatchState(a.state, b.state);
        }
    );

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        glyph_sdf_commands.begin(),
        glyph_sdf_commands.end(),
        [](const DrawCommandGlyphSDF& a, const DrawCommandGlyphSDF& b) {
            return SortTextureBatchState(a.state, b.state);
        }
    );

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        glyph_vector_commands.begin(),
        glyph_vector_commands.end(),
        [](const DrawCommandGlyphVector& a, const DrawCommandGlyphVector& b) {
            if (a.state.order < b.state.order) return true;
            if (a.state.order > b.state.order) return false;

            const int a_scissor_size = a.state.scissor.width() + a.state.scissor.height();
            const int b_scissor_size = b.state.scissor.width() + b.state.scissor.height();

            if (a_scissor_size < b_scissor_size)
                return true;

            if (a_scissor_size > b_scissor_size)
                return false;

            if (a.state.curve_buffer.Get() < b.state.curve_buffer.Get()) return true;
            if (a.state.curve_buffer.Get() > b.state.curve_buffer.Get()) return false;

            uint8_t a_bm = static_cast<uint8_t>(a.state.blend_mode);
            uint8_t b_bm = static_cast<uint8_t>(b.state.blend_mode);

            if (a_bm < b_bm) return true;
            if (a_bm > b_bm) return false;

            return false;
        }
    );

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        ninepatch_commands.begin(),
        ninepatch_commands.end(),
        [](const DrawCommandNinePatch& a, const DrawCommandNinePatch& b) {
            return SortTextureBatchState(a.state, b.state);
        }
    );

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        shape_commands.begin(),
        shape_commands.end(),
        [](const DrawCommandShape& a, const DrawCommandShape& b) {
            return SortSimpleBatchState(a.state, b.state);
        }
    );

    std::sort(
#if PARALLEL
        std::execution::par,
#endif
        line_commands.begin(),
        line_commands.end(),
        [](const DrawCommandLine& a, const DrawCommandLine& b) {
            return SortSimpleBatchState(a.state, b.state);
        }
    );
}

void sge::Renderer2D::UpdateBatchBuffers(sge::Batch& batch) {
    if (batch.empty())
        return;

    ZoneScoped;

    using namespace sge::internal;

    struct batch_data {
        uint32_t count = 0;
        uint32_t offset = 0;
        uint32_t total_count = 0;
    };

    batch_data datas[size_t(FlushDataType::COUNT)] = {};

    uint32_t sum_total_count = 0;

    std::optional<sge::internal::BatchTextureState> sprite_state;
    std::optional<sge::internal::BatchGlyphVectorState> glyph_vector_state;
    std::optional<sge::internal::BatchTextureState> glyph_sdf_state;
    std::optional<sge::internal::BatchTextureState> npatch_state;
    std::optional<sge::internal::BatchSimpleState> shape_state;
    std::optional<sge::internal::BatchSimpleState> line_state;

    const auto& sprites = batch.sprite_draw_commands();
    const auto& glyphs_vector = batch.glyph_vector_draw_commands();
    const auto& glyphs_sdf = batch.glyph_sdf_draw_commands();
    const auto& npatches = batch.ninepatch_draw_commands();
    const auto& shapes = batch.shape_draw_commands();
    const auto& lines = batch.line_draw_commands();

    auto sprite_it = std::next(sprites.cbegin(), batch.sprites_done());
    auto glyph_vector_it = std::next(glyphs_vector.cbegin(), batch.glyphs_vector_done());
    auto glyph_sdf_it = std::next(glyphs_sdf.cbegin(), batch.glyphs_sdf_done());
    auto npatch_it = std::next(npatches.cbegin(), batch.ninepatches_done());
    auto shape_it = std::next(shapes.cbegin(), batch.shapes_done());
    auto line_it = std::next(lines.cbegin(), batch.lines_done());

    auto sprites_end = sprites.cend();
    auto glyphs_vector_end = glyphs_vector.cend();
    auto glyphs_sdf_end = glyphs_sdf.cend();
    auto npatches_end = npatches.cend();
    auto shapes_end = shapes.cend();
    auto lines_end = lines.cend();

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    auto get_next_order = [&]() -> std::optional<uint32_t> {
        std::optional<uint32_t> min;
        auto consider = [&](uint32_t order) {
            if (!min || order < *min) min = order;
        };
        if (sprite_it        != sprites_end)         consider(sprite_it->state.order);
        if (glyph_vector_it  != glyphs_vector_end)   consider(glyph_vector_it->state.order);
        if (glyph_sdf_it     != glyphs_sdf_end)      consider(glyph_sdf_it->state.order);
        if (npatch_it        != npatches_end)        consider(npatch_it->state.order);
        if (shape_it         != shapes_end)          consider(shape_it->state.order);
        if (line_it          != lines_end)           consider(line_it->state.order);
        return min;
    };

    auto flush_sprites = [&]() {
        batch_data& data = datas[size_t(FlushDataType::Sprite)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .texture = sprite_state->texture,
                .scissor = sprite_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::Sprite,
                .blend_mode = sprite_state->blend_mode
            });
            data.count = 0;
            data.offset = data.count;
        }
    };

    auto flush_glyphs_vector = [&]() {
        batch_data& data = datas[size_t(FlushDataType::GlyphVector)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .glyph_data = {
                    .curve_buffer=glyph_vector_state->curve_buffer,
                    .partition_buffer=glyph_vector_state->partition_buffer,
                },
                .scissor = glyph_vector_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::GlyphVector,
                .blend_mode = glyph_vector_state->blend_mode
            });
            data.count = 0;
            data.offset = data.total_count;
        }
    };

    auto flush_glyphs_sdf = [&]() {
        batch_data& data = datas[size_t(FlushDataType::GlyphSDF)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .texture = glyph_sdf_state->texture,
                .scissor = glyph_sdf_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::GlyphSDF,
                .blend_mode = glyph_sdf_state->blend_mode
            });
            data.count = 0;
            data.offset = data.total_count;
        }
    };

    auto flush_npatches = [&]() {
        batch_data& data = datas[size_t(FlushDataType::NinePatch)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .texture = npatch_state->texture,
                .scissor = npatch_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::NinePatch,
                .blend_mode = npatch_state->blend_mode
            });
            data.count = 0;
            data.offset = data.total_count;
        }
    };

    auto flush_shapes = [&]() {
        batch_data& data = datas[size_t(FlushDataType::Shape)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .scissor = shape_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::Shape,
                .blend_mode = shape_state->blend_mode
            });
            data.count = 0;
            data.offset = data.total_count;
        }
    };

    auto flush_lines = [&]() {
        batch_data& data = datas[size_t(FlushDataType::Shape)];
        if (data.count > 0) {
            flush_queue.push_back(FlushData {
                .scissor = line_state->scissor,
                .offset = data.offset,
                .count = data.count,
                .type = FlushDataType::Line,
                .blend_mode = line_state->blend_mode
            });
            data.count = 0;
            data.offset = data.total_count;
        }
    };

    while (sum_total_count < batch.MaxCount()) {
        auto order = get_next_order();
        if (!order) break;

        if (sprite_it != sprites_end && sprite_it->state.order == *order) {
            if (sprite_state && *sprite_state != sprite_it->state) flush_sprites();
            sprite_state = sprite_it->state;

            while (sprite_it != sprites_end && sprite_it->state == *sprite_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }

                const DrawCommandSprite& command = *(sprite_it++);

                uint8_t flags = 0;
                flags |= batch.IsUi() << SpriteFlags::UI;

                SpriteInstance* buffer = m_sprite_batch_data.GetBufferAndAdvance();
                buffer->rotation = command.rotation;
                buffer->uv_offset_scale = command.uv_offset_scale;
                buffer->color = command.color;
                buffer->outline_color = command.outline_color;
                buffer->position = command.position;
                buffer->size = command.size;
                buffer->offset = command.offset;
                buffer->outline_thickness = command.outline_thickness;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::Sprite)].count++;
                datas[size_t(FlushDataType::Sprite)].total_count++;
            }
        }

        if (npatch_it != npatches_end && npatch_it->state.order == *order) {
            if (npatch_state && *npatch_state != npatch_it->state) flush_npatches();
            npatch_state = npatch_it->state;

            while (npatch_it != npatches_end && npatch_it->state == *npatch_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }
                
                const DrawCommandNinePatch& command = *(npatch_it++);
                
                uint8_t flags = 0;
                flags |= batch.IsUi() << SpriteFlags::UI;

                NinePatchInstance* buffer = m_ninepatch_batch_data.GetBufferAndAdvance();
                buffer->rotation = command.rotation;
                buffer->uv_offset_scale = command.uv_offset_scale;
                buffer->color = command.color;
                buffer->margin = command.margin;
                buffer->position = command.position;
                buffer->offset = command.offset;
                buffer->source_size = command.source_size;
                buffer->output_size = command.output_size;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::NinePatch)].count++;
                datas[size_t(FlushDataType::NinePatch)].total_count++;
            }
        }

        if (glyph_sdf_it != glyphs_sdf_end && glyph_sdf_it->state.order == *order) {
            if (glyph_sdf_state && *glyph_sdf_state != glyph_sdf_it->state) flush_glyphs_sdf();
            glyph_sdf_state = glyph_sdf_it->state;

            while (glyph_sdf_it != glyphs_sdf_end && glyph_sdf_it->state == *glyph_sdf_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }

                const DrawCommandGlyphSDF& command = *(glyph_sdf_it++);

                uint8_t flags = 0;
                flags |= batch.IsUi() << ShapeFlags::UI;

                GlyphInstanceSDF* buffer = m_glyph_sdf_batch_data.GetBufferAndAdvance();
                buffer->color = command.color;
                buffer->pos = command.pos;
                buffer->size = command.size;
                buffer->tex_size = command.tex_size;
                buffer->uv = command.tex_uv;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::GlyphSDF)].count++;
                datas[size_t(FlushDataType::GlyphSDF)].total_count++;
            }
        }

        if (glyph_vector_it != glyphs_vector_end && glyph_vector_it->state.order == *order) {
            if (glyph_vector_state && *glyph_vector_state != glyph_vector_it->state) flush_glyphs_vector();
            glyph_vector_state = glyph_vector_it->state;

            while (glyph_vector_it != glyphs_vector_end && glyph_vector_it->state == *glyph_vector_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }

                const DrawCommandGlyphVector& command = *(glyph_vector_it++);

                uint8_t flags = 0;
                flags |= batch.IsUi() << ShapeFlags::UI;

                GlyphInstanceVector* buffer = m_glyph_vector_batch_data.GetBufferAndAdvance();
                buffer->color = command.color;
                buffer->pos = command.pos;
                buffer->size = command.size;
                buffer->em_size = command.em_size;
                buffer->partition_offset = command.partition_offset;
                buffer->partition_count = command.partition_count;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::GlyphVector)].count++;
                datas[size_t(FlushDataType::GlyphVector)].total_count++;
            }
        }

        if (line_it != lines_end && line_it->state.order == *order) {
            if (line_state && *line_state != line_it->state) flush_lines();
            line_state = line_it->state;

            while (line_it != lines_end && line_it->state == *line_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }

                const DrawCommandLine& command = *(line_it++);

                uint8_t flags = 0;
                flags |= batch.IsUi() << ShapeFlags::UI;

                LineInstance* buffer = m_line_batch_data.GetBufferAndAdvance();
                buffer->start = command.start;
                buffer->end = command.end;
                buffer->color = command.color.to_vec4();
                buffer->border_radius = command.border_radius;
                buffer->thickness = command.thickness;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::Line)].count++;
                datas[size_t(FlushDataType::Line)].total_count++;
            }
        }

        if (shape_it != shapes_end && shape_it->state.order == *order) {
            if (shape_state && *shape_state != shape_it->state) flush_shapes();
            shape_state = shape_it->state;
        
            while (shape_it != shapes_end && shape_it->state == *shape_state) {
                if (sum_total_count >= batch.MaxCount()) {
                    goto end;
                }

                const DrawCommandShape& command = *(shape_it++);

                uint8_t flags = 0;
                flags |= batch.IsUi() << ShapeFlags::UI;

                ShapeInstance* buffer = m_shape_batch_data.GetBufferAndAdvance();
                buffer->color = command.color.to_vec4();
                buffer->border_color = command.border_color.to_vec4();
                buffer->border_radius = command.border_radius;
                buffer->position = glm::vec3(command.position, 0.0f);
                buffer->size = command.size;
                buffer->offset = command.offset;
                buffer->border_thickness = command.border_thickness;
                buffer->shape = command.shape;
                buffer->flags = flags;

                ++sum_total_count;
                datas[size_t(FlushDataType::Shape)].count++;
                datas[size_t(FlushDataType::Shape)].total_count++;
            }
        }

// bruh goto in 2026...
end:
        flush_sprites();
        flush_glyphs_vector();
        flush_glyphs_sdf();
        flush_npatches();
        flush_shapes();
        flush_lines();
    }

    batch.sprite_data().total_count       -= datas[size_t(FlushDataType::Sprite)].total_count;
    batch.glyph_vector_data().total_count -= datas[size_t(FlushDataType::GlyphVector)].total_count;
    batch.glyph_sdf_data().total_count    -= datas[size_t(FlushDataType::GlyphSDF)].total_count;
    batch.ninepatch_data().total_count    -= datas[size_t(FlushDataType::NinePatch)].total_count;
    batch.shape_data().total_count        -= datas[size_t(FlushDataType::Shape)].total_count;
    batch.line_data().total_count         -= datas[size_t(FlushDataType::Line)].total_count;
}


void sge::Renderer2D::PrepareBatch(sge::Batch& batch) {
    if (batch.empty()) return;

    const auto& context = GetRenderContext();

    const uint32_t sprite_count = std::min<uint32_t>(batch.sprite_data().total_count, batch.MaxCount());
    const uint32_t glyph_vector_count = std::min<uint32_t>(batch.glyph_vector_data().total_count, batch.MaxCount());
    const uint32_t glyph_sdf_count = std::min<uint32_t>(batch.glyph_sdf_data().total_count, batch.MaxCount());
    const uint32_t ninepatch_count = std::min<uint32_t>(batch.ninepatch_data().total_count, batch.MaxCount());
    const uint32_t shape_count = std::min<uint32_t>(batch.shape_data().total_count, batch.MaxCount());
    const uint32_t line_count = std::min<uint32_t>(batch.line_data().total_count, batch.MaxCount());

    m_sprite_batch_data.ResizeBuffersIfNeeded(*context, m_sprite_batch_data.Count() + sprite_count);
    m_glyph_vector_batch_data.ResizeBuffersIfNeeded(*context, m_glyph_vector_batch_data.Count() + glyph_vector_count);
    m_glyph_sdf_batch_data.ResizeBuffersIfNeeded(*context, m_glyph_sdf_batch_data.Count() + glyph_sdf_count);
    m_ninepatch_batch_data.ResizeBuffersIfNeeded(*context, m_ninepatch_batch_data.Count() + ninepatch_count);
    m_shape_batch_data.ResizeBuffersIfNeeded(*context, m_shape_batch_data.Count() + shape_count);
    m_line_batch_data.ResizeBuffersIfNeeded(*context, m_line_batch_data.Count() + line_count);

    batch.sprite_data().offset = m_sprite_batch_data.Count();
    batch.glyph_vector_data().offset = m_glyph_vector_batch_data.Count();
    batch.glyph_sdf_data().offset = m_glyph_sdf_batch_data.Count();
    batch.ninepatch_data().offset = m_ninepatch_batch_data.Count();
    batch.shape_data().offset = m_shape_batch_data.Count();
    batch.line_data().offset = m_line_batch_data.Count();

    SortBatchDrawCommands(batch);
    UpdateBatchBuffers(batch);
}

void sge::Renderer2D::UploadBatchData() {
    auto* command_buffer = CommandBuffer();

    if (m_sprite_batch_data.Count() > 0) {
        m_sprite_batch_data.Update(*command_buffer);
    }

    if (m_glyph_vector_batch_data.Count() > 0) {
        m_glyph_vector_batch_data.Update(*command_buffer);
    }

    if (m_glyph_sdf_batch_data.Count() > 0) {
        m_glyph_sdf_batch_data.Update(*command_buffer);
    }

    if (m_ninepatch_batch_data.Count() > 0) {
        m_ninepatch_batch_data.Update(*command_buffer);
    }

    if (m_shape_batch_data.Count() > 0) {
        m_shape_batch_data.Update(*command_buffer);
    }

    if (m_line_batch_data.Count() > 0) {
        m_line_batch_data.Update(*command_buffer);
    }
}

void sge::Renderer2D::RenderBatch(sge::Batch& batch) {
    ZoneScoped;

    if (batch.empty()) return;

    ApplyBatchDrawCommands(batch);

    while (batch.total_remaining() > 0) {
        m_batch_instance_count = 0;

        m_sprite_batch_data.Reset();
        batch.sprite_data().offset = 0;

        m_glyph_vector_batch_data.Reset();
        batch.glyph_vector_data().offset = 0;

        m_glyph_sdf_batch_data.Reset();
        batch.glyph_sdf_data().offset = 0;

        m_ninepatch_batch_data.Reset();
        batch.ninepatch_data().offset = 0;

        m_shape_batch_data.Reset();
        batch.shape_data().offset = 0;

        m_line_batch_data.Reset();
        batch.line_data().offset = 0;
        
        UpdateBatchBuffers(batch);
        UploadBatchData();
        
        ApplyBatchDrawCommands(batch);
    }

    batch.Reset();
}

void sge::Renderer2D::InitVectorPipeline() {
    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = m_vector_vertex_format.attributes;

    ShaderSourceCode shader = GetVectorShaderSourceCode(m_context->Backend());
    m_vector_vertex_shader = m_context->CreateShader(sge::ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
    m_vector_fragment_shader = m_context->CreateShader(sge::ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size, shaderConfig);

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
