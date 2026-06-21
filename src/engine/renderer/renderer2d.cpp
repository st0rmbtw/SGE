#include <execution>

#include <SGE/profile.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/binding_layout.hpp>

#include "shaders.hpp"

static constexpr uint32_t DEFAULT_BATCH_COUNT = 2000;

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
    LLGL::VertexFormat vertex_format = sge::Attributes(backend, {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = sge::Attributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_rotation", "I_Rotation", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_outline_color", "I_OutlineColor", 1),
        sge::Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(LLGL::Format::R32Float, "inp_i_outline_thickness", "I_OutlineThickness", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats NinepatchBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format = sge::Attributes(backend, {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = sge::Attributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_rotation", "I_Rotation", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32UInt, "inp_i_margin", "I_Margin", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_source_size", "I_SourceSize", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_output_size", "I_OutputSize", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats GlyphBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format = sge::Attributes(backend, {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = sge::Attributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_tex_size", "I_TexSize", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_uv", "I_UV", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats ShapeBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format = sge::Attributes(backend, {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = sge::Attributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_color", "I_BorderColor", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_radius", "I_BorderRadius", 1),
        sge::Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_position", "I_Position", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        sge::Attribute::Instance(LLGL::Format::R32Float, "inp_i_border_thickness", "I_BorderThickness", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_shape", "I_Shape", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1)
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

BatchVertexFormats LineBatchVertexFormats(const sge::RenderBackend backend) {
    LLGL::VertexFormat vertex_format = sge::Attributes(backend, {
        sge::Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = sge::Attributes(backend, vertex_format.attributes.size(), {
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_start", "I_Start", 1),
        sge::Attribute::Instance(LLGL::Format::RG32Float, "inp_i_end", "I_End", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        sge::Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_radius", "I_Border_Radius", 1),
        sge::Attribute::Instance(LLGL::Format::R32Float, "inp_i_thickness", "I_Thickness", 1),
        sge::Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
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
    m_glyph_vertex_shader = CreateBatchVertexShader(context, GetFontShaderSourceCode(backend), GlyphBatchVertexFormats(backend));
    m_ninepatch_vertex_shader = CreateBatchVertexShader(context, GetNinepatchShaderSourceCode(backend), NinepatchBatchVertexFormats(backend));

    m_sprite_batch_data = InitSpriteBatchData();
    m_ninepatch_batch_data = InitNinepatchBatchData();
    m_glyph_batch_data = InitGlyphBatchData();
    m_shape_batch_data = InitShapeBatchData();
    m_line_batch_data = InitLineBatchData();


    {
        ShaderSourceCode shader = GetSpriteShaderSourceCode(backend);
        m_sprite_default_fragment_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }

    {
        ShaderSourceCode shader = GetFontShaderSourceCode(backend);
        m_glyph_default_fragment_shader = context->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
    }
}

void sge::Renderer2D::Begin() {
    m_batch_instance_count = 0;

    m_sprite_batch_data.Reset();
    m_glyph_batch_data.Reset();
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

sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateGlyphBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader) {
    if (!fragment_shader) {
        fragment_shader = m_glyph_default_fragment_shader;
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
    pipelineConfig.vertexShader = m_glyph_vertex_shader;
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

sge::BatchData<sge::GlyphInstance> sge::Renderer2D::InitGlyphBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = GlyphBatchVertexFormats(GetRenderContext()->Backend());

    BatchData<GlyphInstance> batchData;
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
    const auto glyph_pipeline = batch.GlyphPipeline();
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
    context->DeletePipeline(glyph_pipeline);
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

            case FlushDataType::Glyph:
                commands->SetVertexBufferArray(*m_glyph_batch_data.GetBufferArray());
                commands->SetPipelineState(context->GetOrCreatePipeline(batch.GlyphPipeline()));
                offset = batch.GlyphData().offset;
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

        if (flush_data.texture.is_valid() && prev_texture_id != flush_data.texture.id) {
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

    sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    std::sort(
        std::execution::par,
        draw_commands.begin(),
        draw_commands.end(),
        [](const DrawCommand& a, const DrawCommand& b) {
            const uint32_t a_order = a.order();
            const uint32_t b_order = b.order();

            if (a_order < b_order) return true;
            if (a_order > b_order) return false;

            const int a_scissor_size = a.scissor().width() + a.scissor().height();
            const int b_scissor_size = b.scissor().width() + b.scissor().height();

            if (a_scissor_size < b_scissor_size)
                return true;

            if (a_scissor_size > b_scissor_size)
                return false;

            const TextureWithSampler& a_texture = a.texture();
            const TextureWithSampler& b_texture = b.texture();

            if (a_texture.id < b_texture.id) return true;
            if (a_texture.id > b_texture.id) return false;

            uint8_t a_bm = static_cast<uint8_t>(a.blend_mode());
            uint8_t b_bm = static_cast<uint8_t>(b.blend_mode());

            if (a_bm < b_bm) return true;
            if (a_bm > b_bm) return false;

            return false;
        }
    );
}

void sge::Renderer2D::UpdateBatchBuffers(sge::Batch& batch, size_t begin) {
    ZoneScoped;

    using namespace sge::internal;

    const sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    if (draw_commands.empty()) return;

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    TextureWithSampler sprite_prev_texture;
    BlendMode sprite_prev_blend_mode;
    uint32_t sprite_count = 0;
    uint32_t sprite_offset = 0;
    uint32_t sprite_total_count = 0;

    TextureWithSampler glyph_prev_texture;
    uint32_t glyph_count = 0;
    uint32_t glyph_offset = 0;
    uint32_t glyph_total_count = 0;

    TextureWithSampler ninepatch_prev_texture;
    uint32_t ninepatch_count = 0;
    uint32_t ninepatch_offset = 0;
    uint32_t ninepatch_total_count = 0;

    uint32_t shape_count = 0;
    uint32_t shape_offset = 0;
    uint32_t shape_total_count = 0;

    uint32_t line_count = 0;
    uint32_t line_offset = 0;
    uint32_t line_total_count = 0;

    sge::IRect prev_scissor = draw_commands[begin].scissor();
    uint32_t prev_order = draw_commands[begin].order();

    size_t i = begin;
    for (; i < draw_commands.size(); ++i) {
        if (m_batch_instance_count >= batch.MaxCount()) {
            break;
        }

        const DrawCommand& draw_command = draw_commands[i];

        if (prev_order != draw_command.order() || prev_scissor != draw_command.scissor()) {
            if (sprite_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .scissor = prev_scissor,
                    .offset = sprite_offset,
                    .count = sprite_count,
                    .type = FlushDataType::Sprite,
                    .blend_mode = sprite_prev_blend_mode
                });
                sprite_count = 0;
                sprite_offset = sprite_total_count;
            }

            if (glyph_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .scissor = prev_scissor,
                    .offset = glyph_offset,
                    .count = glyph_count,
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
                glyph_count = 0;
                glyph_offset = glyph_total_count;
            }

            if (ninepatch_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .scissor = prev_scissor,
                    .offset = ninepatch_offset,
                    .count = ninepatch_count,
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_offset = ninepatch_total_count;
            }

            if (shape_count > 0) {
                flush_queue.push_back(FlushData {
                    .scissor = prev_scissor,
                    .offset = shape_offset,
                    .count = shape_count,
                    .type = FlushDataType::Shape,
                    .blend_mode = draw_command.blend_mode()
                });
                shape_count = 0;
                shape_offset = shape_total_count;
            }

            if (line_count > 0) {
                flush_queue.push_back(FlushData {
                    .scissor = prev_scissor,
                    .offset = line_offset,
                    .count = line_count,
                    .type = FlushDataType::Line,
                    .blend_mode = draw_command.blend_mode()
                });
                line_count = 0;
                line_offset = line_total_count;
            }
        }

        switch (draw_command.type()) {
        case DrawCommand::DrawSprite: {
            const DrawCommandSprite& sprite_data = draw_command.sprite_data();

            if (sprite_total_count == 0) {
                sprite_prev_texture = draw_command.texture();
                sprite_prev_blend_mode = draw_command.blend_mode();
            }

            const uint32_t prev_texture_id = sprite_prev_texture.id;
            const uint32_t curr_texture_id = draw_command.texture().id;

            const sge::BlendMode curr_blend_mode = draw_command.blend_mode();

            const bool flush = (prev_texture_id != curr_texture_id || sprite_prev_blend_mode != curr_blend_mode);

            if (sprite_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = sprite_offset,
                    .count = sprite_count,
                    .type = FlushDataType::Sprite,
                    .blend_mode = sprite_prev_blend_mode
                });
                sprite_count = 0;
                sprite_offset = sprite_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.IsUi() << SpriteFlags::UI;

            SpriteInstance* buffer = m_sprite_batch_data.GetBufferAndAdvance();
            buffer->rotation = sprite_data.rotation;
            buffer->uv_offset_scale = sprite_data.uv_offset_scale;
            buffer->color = sprite_data.color;
            buffer->outline_color = sprite_data.outline_color;
            buffer->position = sprite_data.position;
            buffer->size = sprite_data.size;
            buffer->offset = sprite_data.offset;
            buffer->outline_thickness = sprite_data.outline_thickness;
            buffer->flags = flags;

            ++sprite_count;
            ++sprite_total_count;

            sprite_prev_texture = draw_command.texture();
            sprite_prev_blend_mode = draw_command.blend_mode();
        } break;
        case DrawCommand::DrawGlyph: {
            const DrawCommandGlyph& glyph_data = draw_command.glyph_data();

            if (glyph_total_count == 0) {
                glyph_prev_texture = draw_command.texture();
            }

            const bool flush = (glyph_prev_texture.id != draw_command.texture().id);

            if (glyph_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = glyph_offset,
                    .count = glyph_count,
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
                glyph_count = 0;
                glyph_offset = glyph_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.IsUi() << ShapeFlags::UI;

            GlyphInstance* buffer = m_glyph_batch_data.GetBufferAndAdvance();
            buffer->color = glyph_data.color;
            buffer->pos = glyph_data.pos;
            buffer->size = glyph_data.size;
            buffer->tex_size = glyph_data.tex_size;
            buffer->uv = glyph_data.tex_uv;
            buffer->flags = flags;

            ++glyph_count;
            ++glyph_total_count;

            glyph_prev_texture = draw_command.texture();
        } break;
        case DrawCommand::DrawNinePatch: {
            const DrawCommandNinePatch& ninepatch_data = draw_command.ninepatch_data();

            if (ninepatch_total_count == 0) {
                ninepatch_prev_texture = draw_command.texture();
            }

            const uint32_t prev_texture_id = ninepatch_prev_texture.id;
            const uint32_t curr_texture_id = draw_command.texture().id;

            const bool flush = (prev_texture_id != curr_texture_id);

            if (ninepatch_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = ninepatch_offset,
                    .count = ninepatch_count,
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_offset = ninepatch_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.IsUi() << SpriteFlags::UI;

            NinePatchInstance* buffer = m_ninepatch_batch_data.GetBufferAndAdvance();
            buffer->rotation = ninepatch_data.rotation;
            buffer->uv_offset_scale = ninepatch_data.uv_offset_scale;
            buffer->color = ninepatch_data.color;
            buffer->margin = ninepatch_data.margin;
            buffer->position = ninepatch_data.position;
            buffer->offset = ninepatch_data.offset;
            buffer->source_size = ninepatch_data.source_size;
            buffer->output_size = ninepatch_data.output_size;
            buffer->flags = flags;

            ++ninepatch_count;
            ++ninepatch_total_count;

            ninepatch_prev_texture = draw_command.texture();
        } break;
        case DrawCommand::DrawShape: {
            const DrawCommandShape& shape_data = draw_command.shape_data();

            uint8_t flags = 0;
            flags |= batch.IsUi() << ShapeFlags::UI;

            ShapeInstance* buffer = m_shape_batch_data.GetBufferAndAdvance();
            buffer->color = shape_data.color.to_vec4();
            buffer->border_color = shape_data.border_color.to_vec4();
            buffer->border_radius = shape_data.border_radius;
            buffer->position = glm::vec3(shape_data.position, 0.0f);
            buffer->size = shape_data.size;
            buffer->offset = shape_data.offset;
            buffer->border_thickness = shape_data.border_thickness;
            buffer->shape = shape_data.shape;
            buffer->flags = flags;

            ++shape_count;
            ++shape_total_count;
        } break;
        case DrawCommand::DrawLine: {
            const DrawCommandLine& line_data = draw_command.line_data();

            uint8_t flags = 0;
            flags |= batch.IsUi() << ShapeFlags::UI;

            LineInstance* buffer = m_line_batch_data.GetBufferAndAdvance();
            buffer->start = line_data.start;
            buffer->end = line_data.end;
            buffer->color = line_data.color.to_vec4();
            buffer->border_radius = line_data.border_radius;
            buffer->thickness = line_data.thickness;
            buffer->flags = flags;

            ++line_count;
            ++line_total_count;
        } break;
        }

        prev_scissor = draw_command.scissor();
        prev_order = draw_command.order();

        ++m_batch_instance_count;
    }
    

    if (sprite_count > 0) {
        flush_queue.push_back(FlushData {
            .texture = sprite_prev_texture,
            .scissor = prev_scissor,
            .offset = sprite_offset,
            .count = sprite_count,
            .type = FlushDataType::Sprite,
            .blend_mode = sprite_prev_blend_mode
        });
    }

    if (glyph_count > 0) {
        flush_queue.push_back(FlushData {
            .texture = glyph_prev_texture,
            .scissor = prev_scissor,
            .offset = glyph_offset,
            .count = glyph_count,
            .type = FlushDataType::Glyph,
            .blend_mode = draw_commands[i - 1].blend_mode()
        });
    }

    if (ninepatch_count > 0) {
        flush_queue.push_back(FlushData {
            .texture = ninepatch_prev_texture,
            .scissor = prev_scissor,
            .offset = ninepatch_offset,
            .count = ninepatch_count,
            .type = FlushDataType::NinePatch,
            .blend_mode = draw_commands[i - 1].blend_mode()
        });
    }

    if (shape_count > 0) {
        flush_queue.push_back(FlushData {
            .scissor = prev_scissor,
            .offset = shape_offset,
            .count = shape_count,
            .type = FlushDataType::Shape,
            .blend_mode = draw_commands[i - 1].blend_mode()
        });
    }

    if (line_count > 0) {
        flush_queue.push_back(FlushData {
            .scissor = prev_scissor,
            .offset = line_offset,
            .count = line_count,
            .type = FlushDataType::Line,
            .blend_mode = draw_commands[i - 1].blend_mode()
        });
    }

    batch.set_draw_commands_done(i);

    batch.sprite_data().total_count -= sprite_total_count;
    batch.glyph_data().total_count -= glyph_total_count;
    batch.ninepatch_data().total_count -= ninepatch_total_count;
    batch.shape_data().total_count -= shape_total_count;
    batch.line_data().total_count -= line_total_count;
}


void sge::Renderer2D::PrepareBatch(sge::Batch& batch) {
    if (batch.draw_commands().empty()) return;

    const auto& context = GetRenderContext();

    const uint32_t sprite_count = std::min<uint32_t>(batch.sprite_data().total_count, batch.MaxCount());
    const uint32_t glyph_count = std::min<uint32_t>(batch.glyph_data().total_count, batch.MaxCount());
    const uint32_t ninepatch_count = std::min<uint32_t>(batch.ninepatch_data().total_count, batch.MaxCount());
    const uint32_t shape_count = std::min<uint32_t>(batch.shape_data().total_count, batch.MaxCount());
    const uint32_t line_count = std::min<uint32_t>(batch.line_data().total_count, batch.MaxCount());

    m_sprite_batch_data.ResizeBuffersIfNeeded(*context, m_sprite_batch_data.Count() + sprite_count);
    m_glyph_batch_data.ResizeBuffersIfNeeded(*context, m_glyph_batch_data.Count() + glyph_count);
    m_ninepatch_batch_data.ResizeBuffersIfNeeded(*context, m_ninepatch_batch_data.Count() + ninepatch_count);
    m_shape_batch_data.ResizeBuffersIfNeeded(*context, m_shape_batch_data.Count() + shape_count);
    m_line_batch_data.ResizeBuffersIfNeeded(*context, m_line_batch_data.Count() + line_count);

    batch.sprite_data().offset = m_sprite_batch_data.Count();
    batch.glyph_data().offset = m_glyph_batch_data.Count();
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

    if (m_glyph_batch_data.Count() > 0) {
        m_glyph_batch_data.Update(*command_buffer);
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

    const sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    if (draw_commands.empty()) return;

    ApplyBatchDrawCommands(batch);

    while (batch.draw_commands_done() < draw_commands.size()) {
        m_batch_instance_count = 0;

        m_sprite_batch_data.Reset();
        batch.sprite_data().offset = 0;

        m_glyph_batch_data.Reset();
        batch.glyph_data().offset = 0;

        m_ninepatch_batch_data.Reset();
        batch.ninepatch_data().offset = 0;

        m_shape_batch_data.Reset();
        batch.shape_data().offset = 0;

        m_line_batch_data.Reset();
        batch.line_data().offset = 0;
        
        UpdateBatchBuffers(batch, batch.draw_commands_done());
        UploadBatchData();
        
        ApplyBatchDrawCommands(batch);
    }
}