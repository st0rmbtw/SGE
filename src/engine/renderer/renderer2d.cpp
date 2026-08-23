#if __has_include(<execution>)
#include <execution>
#endif

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

// GNU PSTL doesn't compile with exceptions disabled
#if (!defined(__GNUC__) || defined(__cpp_exceptions)) && defined(__cpp_lib_execution)
    #define PARALLEL 1
#else
    #define PARALLEL 0
#endif

static constexpr uint32_t DEFAULT_BATCH_COUNT = 2000;
static constexpr uint32_t VECTOR_VERTEX_BUFFER_SIZE = 10000;

namespace {

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

BatchVertexFormats NinepatchBatchVertexFormats(const sge::RenderBackend backend) {
    // LLGL::VertexFormat vertex_format;
    // vertex_format.attributes = sge::VertexAttributes(backend, {
    //     sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    // });

    // LLGL::VertexFormat instance_format;
    // instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_rotation", "I_Rotation", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint32x4,  "inp_i_margin", "I_Margin", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_source_size", "I_SourceSize", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_output_size", "I_OutputSize", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    // });

    // return {
    //     .vertex = vertex_format,
    //     .instance = instance_format
    // };
}

BatchVertexFormats GlyphVectorBatchVertexFormats(const sge::RenderBackend backend) {
    // LLGL::VertexFormat vertex_format;
    // vertex_format.attributes = sge::VertexAttributes(backend, {
    //     sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    // });
    // LLGL::VertexFormat instance_format;
    // instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_em_size", "I_Em_Size", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint32, "inp_i_partition_offset", "I_PartitionOffset", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint32, "inp_i_partition_count", "I_PartitionCount", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    // });

    // return BatchVertexFormats {
    //     .vertex = vertex_format,
    //     .instance = instance_format
    // };
}

BatchVertexFormats GlyphSDFBatchVertexFormats(const sge::RenderBackend backend) {
    // LLGL::VertexFormat vertex_format;
    // vertex_format.attributes = sge::VertexAttributes(backend, {
    //     sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    // });
    // LLGL::VertexFormat instance_format;
    // instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_color", "I_Color", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_position", "I_Position", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_tex_size", "I_TexSize", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_uv", "I_UV", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1),
    // });

    // return BatchVertexFormats {
    //     .vertex = vertex_format,
    //     .instance = instance_format
    // };
}

BatchVertexFormats ShapeBatchVertexFormats(const sge::RenderBackend backend) {
    // LLGL::VertexFormat vertex_format;
    // vertex_format.attributes = sge::VertexAttributes(backend, {
    //     sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "inp_position", "Position"),
    // });

    // LLGL::VertexFormat instance_format;
    // instance_format.attributes = sge::VertexAttributes(backend, vertex_format.attributes.size(), {
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_color", "I_Color", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_color", "I_BorderColor", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x4, "inp_i_border_radius", "I_BorderRadius", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x3, "inp_i_position", "I_Position", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_size", "I_Size", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32x2, "inp_i_offset", "I_Offset", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Float32, "inp_i_border_thickness", "I_BorderThickness", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_shape", "I_Shape", 1),
    //     sge::Attribute::Instance(sge::VertexFormat::Uint8, "inp_i_flags", "I_Flags", 1)
    // });

    // return BatchVertexFormats {
    //     .vertex = vertex_format,
    //     .instance = instance_format
    // };
}

SGE_FORCE_INLINE sge::Ref<LLGL::Shader> CreateBatchVertexShader(const std::shared_ptr<sge::RenderContext>& context, const ShaderSourceCode& source_code, const BatchVertexFormats& vertex_formats) {
    sge::ShaderConfig shaderConfig;
    shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();
    return context->CreateShader(sge::ShaderType::Vertex, "VS", source_code.vs_source, source_code.vs_size, shaderConfig);
}

SGE_FORCE_INLINE sge::Handle<LLGL::PipelineState> GetPipelineByBlendMode(sge::BlendMode blend_mode, const sge::SpriteBatchPipeline& pipeline) {

}

SGE_FORCE_INLINE sge::Handle<LLGL::PipelineState> GetDepthPipelineByBlendMode(sge::BlendMode blend_mode, const sge::SpriteBatchPipeline& pipeline) {

}

} // namespace

sge::Renderer2D::Renderer2D(const std::shared_ptr<RenderContext>& context) : Renderer(context) {
    const RenderBackend backend = context->Backend();

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

    m_vector_vertex_format = sge::VertexAttributes(m_context->Backend(), {
        sge::Attribute::Vertex(sge::VertexFormat::Float32x2, "a_position", "Position")
    });
    m_vector_vertex_buffer = m_context->CreateVertexBuffer(VECTOR_VERTEX_BUFFER_SIZE * sizeof(glm::vec2), m_vector_vertex_format, "Vector Vertex Buffer");
}

// sge::SpriteBatchPipeline sge::Renderer2D::CreateSpriteBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader) {
//     if (!fragment_shader) {
//         fragment_shader = m_sprite_default_fragment_shader;
//     }

//     static uint32_t count = 0;

//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//         BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
//         BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
//     });

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.vertexShader = m_sprite_vertex_shader;
//     pipelineConfig.pixelShader = fragment_shader;
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;

//     LLGL::BlendDescriptor blend_modes[4] = {
//         // AlphaBlend
//         LLGL::BlendDescriptor {
//             .targets = {
//                 LLGL::BlendTargetDescriptor {
//                     .blendEnabled = true,
//                     .srcColor = LLGL::BlendOp::SrcAlpha,
//                     .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                     .srcAlpha = LLGL::BlendOp::SrcAlpha,
//                     .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
//                 }
//             }
//         },
//         // Additive
//         LLGL::BlendDescriptor {
//             .targets = {
//                 LLGL::BlendTargetDescriptor {
//                     .blendEnabled = true,
//                     .srcColor = LLGL::BlendOp::SrcAlpha,
//                     .dstColor = LLGL::BlendOp::One,
//                     .srcAlpha = LLGL::BlendOp::SrcAlpha,
//                     .dstAlpha = LLGL::BlendOp::One,
//                 }
//             }
//         },
//         // Opaque
//         LLGL::BlendDescriptor {
//             .targets = {
//                 LLGL::BlendTargetDescriptor {
//                     .blendEnabled = true,
//                     .srcColor = LLGL::BlendOp::One,
//                     .dstColor = LLGL::BlendOp::Zero,
//                     .srcAlpha = LLGL::BlendOp::One,
//                     .dstAlpha = LLGL::BlendOp::Zero,
//                 }
//             }
//         },
//         // PremultipliedAlpha
//         LLGL::BlendDescriptor {
//             .targets = {
//                 LLGL::BlendTargetDescriptor {
//                     .blendEnabled = true,
//                     .srcColor = LLGL::BlendOp::One,
//                     .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                     .srcAlpha = LLGL::BlendOp::One,
//                     .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
//                 }
//             }
//         }
//     };

//     SpriteBatchPipeline pipeline;

//     {
//         std::tuple<sge::BlendMode, sge::Handle<LLGL::PipelineState>*, const char*, int> pipelines[4] = {
//             {
//                 sge::BlendMode::AlphaBlend, &pipeline.alpha_blend, "SpriteBatchPipelineAlphaBlend", 0
//             },
//             {
//                 sge::BlendMode::Additive, &pipeline.additive, "SpriteBatchPipelineAdditive", 1
//             },
//             {
//                 sge::BlendMode::Opaque, &pipeline.opaque, "SpriteBatchPipelineOpaque", 2
//             },
//             {
//                 sge::BlendMode::PremultipliedAlpha, &pipeline.premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlpha", 3
//             },
//         };

//         for (const auto& [blend_mode, pointer, pipelineName, index] : pipelines) {
//             pipelineConfig.debugName = std::format("{}_{}", pipelineName, count);

//             // bool hasInitialCache = false;
//             // auto pipelineCache = ReadPipelineCache(name, hasInitialCache);

//             pipelineConfig.blend = blend_modes[index];
//             // LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache.get());
//             *pointer = GetRenderContext()->CreatePipelineState(pipelineConfig);

//             // if (!hasInitialCache) {
//             //     SavePipelineCache(name, *pipelineCache);
//             // }

//             // if (const LLGL::Report* report = pipeline->GetReport()) {
//             //     if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
//             // }
//         }
//     }
//     {
//         std::tuple<sge::BlendMode, sge::Handle<LLGL::PipelineState>*, const char*, int> pipelines[4] = {
//             {
//                 sge::BlendMode::AlphaBlend, &pipeline.depth_alpha_blend, "SpriteBatchPipelineAlphaBlendDepth", 0
//             },
//             {
//                 sge::BlendMode::Additive, &pipeline.depth_additive, "SpriteBatchPipelineAdditiveDepth", 1
//             },
//             {
//                 sge::BlendMode::Opaque, &pipeline.depth_opaque, "SpriteBatchPipelineOpaqueDepth", 2
//             },
//             {
//                 sge::BlendMode::PremultipliedAlpha, &pipeline.depth_premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlphaDepth", 3
//             },
//         };

//         GraphicsPipelineConfig depthPipelineConfig = pipelineConfig;
//         depthPipelineConfig.depth = LLGL::DepthDescriptor {
//             .testEnabled = true,
//             .writeEnabled = true,
//             .compareOp = LLGL::CompareOp::GreaterEqual,
//         };

//         for (const auto& [blend_mode, pointer, pipelineName, index] : pipelines) {
//             depthPipelineConfig.debugName = pipelineName;
//             *pointer = GetRenderContext()->CreatePipelineState(depthPipelineConfig);
//         }
//     }

//     ++count;

//     return pipeline;
// }

// sge::Handle<LLGL::PipelineState> CreateNinepatchBatchPipeline(bool enable_scissor) {
//     const RenderBackend backend = GetRenderContext()->Backend();

//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//         BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
//         BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
//     });

//     ShaderSourceCode shader = GetNinepatchShaderSourceCode(backend);

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.debugName = "NinePatchBatch Pipeline";
//     pipelineConfig.vertexShader = m_ninepatch_vertex_shader;
//     pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;
//     pipelineConfig.blend = LLGL::BlendDescriptor {
//         .targets = {
//             LLGL::BlendTargetDescriptor {
//                 .blendEnabled = true,
//                 .srcColor = LLGL::BlendOp::SrcAlpha,
//                 .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                 .srcAlpha = LLGL::BlendOp::Zero,
//                 .dstAlpha = LLGL::BlendOp::One,
//                 .alphaArithmetic = LLGL::BlendArithmetic::Max
//             }
//         }
//     };

//     return GetRenderContext()->CreatePipelineState(pipelineConfig);
// }

// sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateGlyphSDFBatchPipeline(bool enable_scissor, Ref<LLGL::Shader> fragment_shader) {
//     if (!fragment_shader) {
//         fragment_shader = m_glyph_sdf_default_fragment_shader;
//     }

//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//         BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
//         BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
//     });
//     pipelineLayoutDesc.combinedTextureSamplers = {
//         LLGL::CombinedTextureSamplerDescriptor{ "Texture", "Texture", "Sampler", 3 }
//     };

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.debugName = "GlyphBatch Pipeline";
//     pipelineConfig.vertexShader = m_glyph_sdf_vertex_shader;
//     pipelineConfig.pixelShader = fragment_shader;
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;
//     pipelineConfig.blend = LLGL::BlendDescriptor {
//         .targets = {
//             LLGL::BlendTargetDescriptor {
//                 .blendEnabled = true,
//                 .srcColor = LLGL::BlendOp::SrcAlpha,
//                 .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                 .srcAlpha = LLGL::BlendOp::Zero,
//                 .dstAlpha = LLGL::BlendOp::One,
//                 .alphaArithmetic = LLGL::BlendArithmetic::Max
//             }
//         }
//     };

//     return GetRenderContext()->CreatePipelineState(pipelineConfig);
// }

// sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateGlyphVectorBatchPipeline(bool enable_scissor) {
//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//         BindingLayoutItem::Buffer(3, "CurveBuffer", LLGL::StageFlags::FragmentStage),
//         BindingLayoutItem::Buffer(4, "PartitionBuffer", LLGL::StageFlags::FragmentStage),
//     });
//     pipelineLayoutDesc.combinedTextureSamplers = {
//         LLGL::CombinedTextureSamplerDescriptor{ "Texture", "Texture", "Sampler", 3 }
//     };

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.debugName = "GlyphBatch Pipeline";
//     pipelineConfig.vertexShader = m_glyph_vector_vertex_shader;
//     pipelineConfig.pixelShader = m_glyph_vector_fragment_shader;
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;
//     pipelineConfig.blend = LLGL::BlendDescriptor {
//         .targets = {
//             LLGL::BlendTargetDescriptor {
//                 .blendEnabled = true,
//                 .srcColor = LLGL::BlendOp::SrcAlpha,
//                 .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                 .srcAlpha = LLGL::BlendOp::Zero,
//                 .dstAlpha = LLGL::BlendOp::One,
//                 .alphaArithmetic = LLGL::BlendArithmetic::Max
//             }
//         }
//     };

//     return GetRenderContext()->CreatePipelineState(pipelineConfig);
// }

// sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateShapeBatchPipeline(bool enable_scissor) {
//     const RenderBackend backend = GetRenderContext()->Backend();

//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//     });

//     ShaderSourceCode shader = GetShapeShaderSourceCode(backend);
//     BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(backend);

//     sge::ShaderConfig shaderConfig;
//     shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.debugName = "ShapeBatch Pipeline";
//     pipelineConfig.vertexShader = GetRenderContext()->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
//     pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;
//     pipelineConfig.blend = LLGL::BlendDescriptor {
//         .targets = {
//             LLGL::BlendTargetDescriptor {
//                 .blendEnabled = true,
//                 .srcColor = LLGL::BlendOp::SrcAlpha,
//                 .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                 .srcAlpha = LLGL::BlendOp::Zero,
//                 .dstAlpha = LLGL::BlendOp::One,
//                 .alphaArithmetic = LLGL::BlendArithmetic::Max
//             }
//         }
//     };

//     return GetRenderContext()->CreatePipelineState(pipelineConfig);
// }

// sge::Handle<LLGL::PipelineState> sge::Renderer2D::CreateLineBatchPipeline(bool enable_scissor) {
//     const RenderBackend backend = GetRenderContext()->Backend();

//     LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
//     pipelineLayoutDesc.bindings = BindingLayout({
//         BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
//     });

//     ShaderSourceCode shader = GetLineShaderSourceCode(backend);
//     BatchVertexFormats vertex_formats = LineBatchVertexFormats(backend);

//     sge::ShaderConfig shaderConfig;
//     shaderConfig.vertex.inputAttribs = vertex_formats.total_attributes();

//     GraphicsPipelineConfig pipelineConfig;
//     pipelineConfig.debugName = "LineBatch Pipeline";
//     pipelineConfig.layout = GetRenderContext()->CreatePipelineLayout(pipelineLayoutDesc);
//     pipelineConfig.vertexShader = GetRenderContext()->CreateShader(ShaderType::Vertex, "VS", shader.vs_source, shader.vs_size, shaderConfig);
//     pipelineConfig.pixelShader = GetRenderContext()->CreateShader(ShaderType::Fragment, "PS", shader.fs_source, shader.fs_size);
//     pipelineConfig.primitiveTopology = sge::PrimitiveTopology::TriangleStrip;
//     pipelineConfig.scissorTestEnabled = enable_scissor;
//     pipelineConfig.blend = LLGL::BlendDescriptor {
//         .targets = {
//             LLGL::BlendTargetDescriptor {
//                 .blendEnabled = true,
//                 .srcColor = LLGL::BlendOp::SrcAlpha,
//                 .dstColor = LLGL::BlendOp::InvSrcAlpha,
//                 .srcAlpha = LLGL::BlendOp::Zero,
//                 .dstAlpha = LLGL::BlendOp::One,
//                 .alphaArithmetic = LLGL::BlendArithmetic::Max
//             }
//         }
//     };

//     return GetRenderContext()->CreatePipelineState(pipelineConfig);
// }

// void sge::Renderer2D::DestroyBatch(sge::Batch& batch) {
//     const SpriteBatchPipeline& sprite_pipeline = batch.SpritePipeline();
//     const auto glyph_vector_pipeline = batch.GlyphVectorPipeline();
//     const auto glyph_sdf_pipeline = batch.GlyphSDFPipeline();
//     const auto line_pipeline = batch.LinePipeline();
//     const auto shape_pipeline = batch.ShapePipeline();

//     auto& context = GetRenderContext();

//     context->DeletePipeline(sprite_pipeline.additive);
//     context->DeletePipeline(sprite_pipeline.alpha_blend);
//     context->DeletePipeline(sprite_pipeline.premultiplied_alpha);
//     context->DeletePipeline(sprite_pipeline.opaque);
//     context->DeletePipeline(sprite_pipeline.depth_additive);
//     context->DeletePipeline(sprite_pipeline.depth_alpha_blend);
//     context->DeletePipeline(sprite_pipeline.depth_premultiplied_alpha);
//     context->DeletePipeline(sprite_pipeline.depth_opaque);
//     context->DeletePipeline(glyph_vector_pipeline);
//     context->DeletePipeline(glyph_sdf_pipeline);
//     context->DeletePipeline(line_pipeline);
//     context->DeletePipeline(shape_pipeline);
// }

static bool BatchStateEqual(const sge::internal::BatchState& a, const sge::internal::BatchState& b, LLGL::Resource* const* resources) {
    size_t hash_a = sge::DEFAULT_HASH;
    sge::hash_fnv1a(hash_a, resources + a.resources_offset, a.resources_count);

    size_t hash_b = sge::DEFAULT_HASH;
    sge::hash_fnv1a(hash_b, resources + b.resources_offset, b.resources_count);

    return hash_a == hash_b
        && a.scissor == b.scissor
        && a.order == b.order
        && a.blend_mode == b.blend_mode;
}

void sge::Renderer2D::SubmitBatchRaw(
    LLGL::PipelineState* pipeline,
    LLGL::Buffer* vertexBuffer,
    sge::VertexBufferPool& instanceBuffer,
    sge::Unique<LLGL::BufferArray>& bufferArray,
    std::vector<DrawCommand>& drawCommands,
    LLGL::Resource* const* resources,
    uint32_t vertexCount,
    void* instanceData
) {
    if (drawCommands.empty())
        return;

    m_indices.clear();
    m_indices.reserve(drawCommands.size());
    std::ranges::copy(std::views::iota(0u, drawCommands.size()), std::back_inserter(m_indices));

    std::ranges::sort(m_indices, [&](const size_t ia, const size_t ib) {
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

        size_t hash_a = sge::DEFAULT_HASH;
        sge::hash_fnv1a(hash_a, resources + a.state.resources_offset, a.state.resources_count);

        size_t hash_b = sge::DEFAULT_HASH;
        sge::hash_fnv1a(hash_b, resources + b.state.resources_offset, b.state.resources_count);

        if (hash_a < hash_b) return true;
        if (hash_a > hash_b) return false;

        return false;
    });

    static constexpr size_t TEMP_BUFFER_SIZE = 256;

    auto* instance_ptr = static_cast<uint8_t*>(instanceData);
    const auto instance_stride = instanceBuffer.GetStride();
    const size_t instances_size = instance_stride * drawCommands.size();

    SGE_ASSERT(instance_stride <= TEMP_BUFFER_SIZE);

    uint8_t temp_buf[TEMP_BUFFER_SIZE];

    // Sort instances and draw commands
    for (size_t i = 0; i < m_indices.size(); ++i) {
        size_t current_idx = i;

        while (m_indices[current_idx] != i) {
            size_t next_idx = m_indices[current_idx];

            std::swap(drawCommands[current_idx], drawCommands[next_idx]);

            auto* elem_curr = instance_ptr + (current_idx * instance_stride);
            auto* elem_next = instance_ptr + (next_idx * instance_stride);

            std::memcpy(temp_buf, elem_curr, instance_stride);
            std::memcpy(elem_curr, elem_next, instance_stride);
            std::memcpy(elem_next, temp_buf, instance_stride);

            m_indices[current_idx] = current_idx;
            current_idx = next_idx;
        }
        m_indices[current_idx] = current_idx;
    }

    if (instanceBuffer.Reserve(*m_context, instances_size)) {
        bufferArray = m_context->CreateBufferArray({ vertexBuffer, instanceBuffer.Get() });
    }

    m_command_buffer->UpdateBuffer(*instanceBuffer.Get(), 0, instanceData, instances_size);

    m_submissions.push_back(BatchSubmission {
        .pipeline = pipeline,
        .buffer_array = bufferArray,
        .resources = resources,
        .draw_commands = drawCommands.data(),
        .draw_commands_count = drawCommands.size(),
        .draw_commands_offset = 0,
        .vertex_count = vertexCount
    });
}

void sge::Renderer2D::FlushBatches() {
    auto get_next_order = [&]() -> std::optional<uint32_t> {
        std::optional<uint32_t> min;
        auto consider = [&](uint32_t order) {
            if (!min || order < *min) min = order;
        };

        for (const auto& s : m_submissions) {
            if (s.draw_commands_offset < s.draw_commands_count) {
                consider(s.draw_commands[s.draw_commands_offset].state.order);
            }
        }

        return min;
    };

    auto FlushBatch = [&](const BatchSubmission& data, const sge::internal::BatchState& state, uint32_t instance_count, uint32_t instance_offset) {
        if (instance_count == 0) return;

        m_command_buffer->SetPipelineState(*data.pipeline);
        m_command_buffer->SetVertexBufferArray(*data.buffer_array);

        if (state.scissor.width() > 0 && state.scissor.height() > 0) {
            m_command_buffer->SetScissor(LLGL::Scissor(
                state.scissor.min.x,
                state.scissor.min.y,
                state.scissor.width(),
                state.scissor.height()
            ));
        } else {
            m_command_buffer->SetScissor(LLGL::Scissor(0, 0, m_viewport.width, m_viewport.height));
        }

        m_command_buffer->SetResource(0, *GlobalUniformBuffer());
        size_t binding = 1;
        for (size_t i = 0; i < state.resources_count; ++i) {
            m_command_buffer->SetResource(binding++, *data.resources[state.resources_offset + i]);
        }

        m_command_buffer->DrawInstanced(data.vertex_count, 0, instance_count, instance_offset);
    };

    m_batch_data.clear();
    for (const auto& submission : m_submissions) {
        m_batch_data.push_back(BatchData {
            .state = submission.draw_commands[0].state,
            .offset = 0
        });
    }

    while (true) {
        auto current_order = get_next_order();
        if (!current_order)
            break;

        for (size_t submission_index = 0; submission_index < m_submissions.size(); ++submission_index) {
            auto& submission = m_submissions[submission_index];
            auto& data = m_batch_data[submission_index];

            uint32_t count = 0;

            size_t i = submission.draw_commands_offset;
            if (submission.draw_commands[i].state.order == current_order) {
                data.state = submission.draw_commands[i].state;

                for (; i < submission.draw_commands_count; ++i) {
                    const DrawCommand& command = submission.draw_commands[i];

                    if (!BatchStateEqual(command.state, data.state, submission.resources))
                        break;

                    ++count;
                }
            }
            submission.draw_commands_offset = i;

            if (count > 0) {
                FlushBatch(submission, data.state, count, data.offset);
                data.offset += count;
                count = 0;
            }
        }
    }

    m_submissions.clear();
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
