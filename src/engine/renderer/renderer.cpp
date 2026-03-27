#include <cstddef>
#include <optional>
#include <utility>

#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/assert.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/utils/alloc.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/math/rect.hpp>

#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/Format.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Types.h>
#include <LLGL/PipelineCache.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/ShaderFlags.h>

#include "SGE/renderer/context.hpp"
#include "shaders.hpp"

using namespace sge;
using namespace sge::internal;

namespace fs = std::filesystem;

static constexpr size_t MAX_QUADS = 2500;

namespace SpriteFlags {
    enum : uint8_t {
        UI = 0,
        IgnoreCameraZoom,
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

static BatchVertexFormats SpriteBatchVertexFormats(const RenderBackend backend) {
    LLGL::VertexFormat vertex_format = Attributes(backend, {
        Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = Attributes(backend, vertex_format.attributes.size(), {
        Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_rotation", "I_Rotation", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_outline_color", "I_OutlineColor", 1),
        Attribute::Instance(LLGL::Format::R32Float, "inp_i_outline_thickness", "I_OutlineThickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

static BatchVertexFormats NinepatchBatchVertexFormats(const RenderBackend backend) {
    LLGL::VertexFormat vertex_format = Attributes(backend, {
        Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = Attributes(backend, vertex_format.attributes.size(), {
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_rotation", "I_Rotation", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_source_size", "I_SourceSize", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_output_size", "I_OutputSize", 1),
        Attribute::Instance(LLGL::Format::RGBA32UInt, "inp_i_margin", "I_Margin", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_uv_offset_scale", "I_UvOffsetScale", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

static BatchVertexFormats GlyphBatchVertexFormats(const RenderBackend backend) {
    LLGL::VertexFormat vertex_format = Attributes(backend, {
        Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = Attributes(backend, vertex_format.attributes.size(), {
        Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_tex_size", "I_TexSize", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_uv", "I_UV", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

static BatchVertexFormats ShapeBatchVertexFormats(const RenderBackend backend) {
    LLGL::VertexFormat vertex_format = Attributes(backend, {
        Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = Attributes(backend, vertex_format.attributes.size(), {
        Attribute::Instance(LLGL::Format::RGB32Float, "inp_i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_color", "I_BorderColor", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_radius", "I_BorderRadius", 1),
        Attribute::Instance(LLGL::Format::R32Float, "inp_i_border_thickness", "I_BorderThickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_shape", "I_Shape", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1)
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

static BatchVertexFormats LineBatchVertexFormats(const RenderBackend backend) {
    LLGL::VertexFormat vertex_format = Attributes(backend, {
        Attribute::Vertex(LLGL::Format::RG32Float, "inp_position", "Position"),
    });
    LLGL::VertexFormat instance_format = Attributes(backend, vertex_format.attributes.size(), {
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_start", "I_Start", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "inp_i_end", "I_End", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "inp_i_border_radius", "I_Border_Radius", 1),
        Attribute::Instance(LLGL::Format::R32Float, "inp_i_thickness", "I_Thickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "inp_i_flags", "I_Flags", 1),
    });

    return BatchVertexFormats {
        .vertex = vertex_format,
        .instance = instance_format
    };
}

template <typename T>
void BatchData<T>::Init(const std::shared_ptr<sge::RenderContext>& context, uint32_t size, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format) {
    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    m_buffer = checked_alloc<T>(size);
    m_buffer_ptr = m_buffer;

    m_vertex_buffer = context->CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "SpriteBatch VertexBuffer");
    m_instance_buffer = context->CreateVertexBuffer(size * sizeof(T), instance_format, "SpriteBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer.get(), m_instance_buffer.get() };
    m_buffer_array = context->Context()->CreateBufferArray(2, buffers);
}

template <typename T>
void BatchData<T>::Destroy(const LLGL::RenderSystemPtr& context) {
    SGE_RESOURCE_RELEASE(m_vertex_buffer);
    SGE_RESOURCE_RELEASE(m_instance_buffer);
    SGE_RESOURCE_RELEASE(m_buffer_array);
    free(m_buffer);

    m_buffer = nullptr;
    m_buffer_ptr = nullptr;
}

static LLGL::Shader* CreateShader(const std::shared_ptr<RenderContext>& context, ShaderType shader_type, const void* data, size_t length, const char* entry_point, std::vector<LLGL::VertexAttribute> vertex_attributes = {}) {
    const RenderBackend backend = context->Backend();

    const char* shader_source = static_cast<const char*>(data);

    LLGL::ShaderDescriptor shader_desc;
    shader_desc.type = shader_type.ToLLGLType();
    shader_desc.source = shader_source;
    shader_desc.sourceSize = length;

    if (shader_type.IsVertex()) {
        shader_desc.vertex.inputAttribs = vertex_attributes;
    }

    if (backend.IsOpenGL() && shader_type.IsFragment()) {
        shader_desc.fragment.outputAttribs = {
            { "frag_color", LLGL::Format::RGBA8UNorm, 0, LLGL::SystemValue::Color }
        };
    }

    if (backend.IsVulkan()) {
        shader_desc.sourceType = LLGL::ShaderSourceType::BinaryBuffer;
    } else {
        shader_desc.sourceType = LLGL::ShaderSourceType::CodeString;
        shader_desc.entryPoint = entry_point;
        shader_desc.profile = shader_type.Profile(backend);
    }

#if SGE_DEBUG
    shader_desc.flags |= LLGL::ShaderCompileFlags::NoOptimization;
#else
    shader_desc.flags |= LLGL::ShaderCompileFlags::OptimizationLevel3;
#endif

    LLGL::Shader* shader = context->Context()->CreateShader(shader_desc);
    if (const LLGL::Report* report = shader->GetReport()) {
        if (*report->GetText() != '\0') {
            if (report->HasErrors()) {
                SGE_LOG_ERROR("Failed to create a shader: {}", report->GetText());
                return nullptr;
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    return shader;
}

SpriteBatchPipeline Renderer::CreateSpriteBatchPipeline(bool enable_scissor, LLGL::Shader* fragment_shader) {
    if (fragment_shader == nullptr) {
        fragment_shader = m_sprite_default_fragment_shader.get();
    }

    static uint32_t count = 0;

    auto& context = m_context->Context();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.vertexShader = m_sprite_vertex_shader.get();
    pipelineConfig.pixelShader = fragment_shader;
    pipelineConfig.layout = pipelineLayout;
    pipelineConfig.indexFormat = LLGL::Format::R16UInt;
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.frontCCW = true;
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
        std::tuple<sge::BlendMode, std::optional<uint32_t>*, const char*, int> pipelines[4] = {
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
            const std::string name = std::format("{}_{}", pipelineName, count);
            pipelineConfig.debugName = name.c_str();
            
            // bool hasInitialCache = false;
            // auto pipelineCache = ReadPipelineCache(name, hasInitialCache);

            pipelineConfig.blend = blend_modes[index];
            // LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache.get());
            *pointer = m_context->AddPipelineConfig(pipelineConfig);

            // if (!hasInitialCache) {
            //     SavePipelineCache(name, *pipelineCache);
            // }

            // if (const LLGL::Report* report = pipeline->GetReport()) {
            //     if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            // }
        }
    }
    {
        std::tuple<sge::BlendMode, std::optional<uint32_t>*, const char*, int> pipelines[4] = {
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
            *pointer = m_context->AddPipelineConfig(pipelineConfig);
        }
    }

    ++count;

    return pipeline;
}

uint32_t Renderer::CreateNinepatchBatchPipeline(bool enable_scissor) {
    auto& context = m_context->Context();
    const RenderBackend backend = m_context->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetNinepatchShaderSourceCode(backend);
    LLGL::Shader* fragment_shader = CreateShader(m_context, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "NinePatchBatch Pipeline";
    pipelineConfig.vertexShader = m_ninepatch_vertex_shader.get();
    pipelineConfig.pixelShader = fragment_shader;
    pipelineConfig.layout = pipelineLayout;
    pipelineConfig.indexFormat = LLGL::Format::R16UInt;
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.frontCCW = true;
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

    return m_context->AddPipelineConfig(pipelineConfig);
}

uint32_t Renderer::CreateGlyphBatchPipeline(bool enable_scissor, LLGL::Shader* fragment_shader) {
    if (fragment_shader == nullptr) {
        fragment_shader = m_glyph_default_fragment_shader.get();
    }

    auto& context = m_context->Context();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });
    pipelineLayoutDesc.combinedTextureSamplers = {
        LLGL::CombinedTextureSamplerDescriptor{ "Texture", "Texture", "Sampler", 3 }
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "GlyphBatch Pipeline";
    pipelineConfig.vertexShader = m_glyph_vertex_shader.get();
    pipelineConfig.pixelShader = fragment_shader;
    pipelineConfig.layout = pipelineLayout;
    pipelineConfig.indexFormat = LLGL::Format::R16UInt;
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.frontCCW = true;
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

    return m_context->AddPipelineConfig(std::move(pipelineConfig));
}

uint32_t Renderer::CreateShapeBatchPipeline(bool enable_scissor) {
    auto& context = m_context->Context();
    const RenderBackend backend = m_context->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetShapeShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(backend);

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "ShapeBatch Pipeline";
    pipelineConfig.vertexShader = CreateShader(m_context, ShaderType::Vertex, shader.vs_source, shader.vs_size, "VS", vertex_formats.total_attributes());
    pipelineConfig.pixelShader = CreateShader(m_context, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    pipelineConfig.layout = pipelineLayout;
    pipelineConfig.indexFormat = LLGL::Format::R16UInt;
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.frontCCW = true;
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

    return m_context->AddPipelineConfig(std::move(pipelineConfig));
}

uint32_t Renderer::CreateLineBatchPipeline(bool enable_scissor) {
    auto& context = m_context->Context();
    const RenderBackend backend = m_context->Backend();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetLineShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = LineBatchVertexFormats(backend);

    GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.debugName = "LineBatch Pipeline";
    pipelineConfig.layout = pipelineLayout;
    pipelineConfig.vertexShader = CreateShader(m_context, ShaderType::Vertex, shader.vs_source, shader.vs_size, "VS", vertex_formats.total_attributes());
    pipelineConfig.pixelShader = CreateShader(m_context, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    pipelineConfig.indexFormat = LLGL::Format::R16UInt;
    pipelineConfig.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineConfig.frontCCW = true;
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

    return m_context->AddPipelineConfig(std::move(pipelineConfig));;
}


BatchData<SpriteInstance> Renderer::InitSpriteBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = SpriteBatchVertexFormats(m_context->Backend());

    BatchData<SpriteInstance> batchData;
    batchData.Init(m_context, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

BatchData<NinePatchInstance> Renderer::InitNinepatchBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = NinepatchBatchVertexFormats(m_context->Backend());

    BatchData<NinePatchInstance> batchData;
    batchData.Init(m_context, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

BatchData<GlyphInstance> Renderer::InitGlyphBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = GlyphBatchVertexFormats(m_context->Backend());

    BatchData<GlyphInstance> batchData;
    batchData.Init(m_context, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

BatchData<ShapeInstance> Renderer::InitShapeBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(m_context->Backend());

    BatchData<ShapeInstance> batchData;
    batchData.Init(m_context, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

BatchData<LineInstance> Renderer::InitLineBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = LineBatchVertexFormats(m_context->Backend());

    BatchData<LineInstance> batchData;
    batchData.Init(m_context, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    return batchData;
}

static SGE_FORCE_INLINE LLGL::Shader* CreateBatchVertexShader(const std::shared_ptr<sge::RenderContext> context, const ShaderSourceCode& source_code, const BatchVertexFormats& vertex_formats) {
    return CreateShader(context, ShaderType::Vertex, source_code.vs_source, source_code.vs_size, "VS", vertex_formats.total_attributes());
}

Renderer::Renderer(const std::shared_ptr<RenderContext>& context) : m_context(context) {
    SGE_ASSERT(context->Context() != nullptr);

    const RenderBackend backend = context->Backend();

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = m_context->Context()->CreateCommandBuffer(command_buffer_desc);
    m_command_queue = m_context->Context()->GetCommandQueue();

    m_constant_buffer = m_context->CreateConstantBuffer(sizeof(GlobalUniforms), "ConstantBuffer");

    m_sprite_vertex_shader = CreateBatchVertexShader(m_context, GetSpriteShaderSourceCode(backend), SpriteBatchVertexFormats(backend));
    m_glyph_vertex_shader = CreateBatchVertexShader(m_context, GetFontShaderSourceCode(backend), GlyphBatchVertexFormats(backend));
    m_ninepatch_vertex_shader = CreateBatchVertexShader(m_context, GetNinepatchShaderSourceCode(backend), NinepatchBatchVertexFormats(backend));

    m_sprite_batch_data = InitSpriteBatchData();
    m_ninepatch_batch_data = InitNinepatchBatchData();
    m_glyph_batch_data = InitGlyphBatchData();
    m_shape_batch_data = InitShapeBatchData();
    m_line_batch_data = InitLineBatchData();


    {
        ShaderSourceCode shader = GetSpriteShaderSourceCode(backend);
        m_sprite_default_fragment_shader = CreateShader(m_context, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    }

    {
        ShaderSourceCode shader = GetFontShaderSourceCode(backend);
        m_glyph_default_fragment_shader = CreateShader(m_context, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    }
}

void Renderer::Begin() {
    ZoneScoped;

    m_batch_instance_count = 0;

    m_sprite_batch_data.Reset();
    m_glyph_batch_data.Reset();
    m_ninepatch_batch_data.Reset();
    m_shape_batch_data.Reset();
    m_line_batch_data.Reset();
}

void Renderer::BeginPass(LLGL::RenderTarget& target, const Camera& camera) {
    m_context->SetCurrentRenderTarget(&target);
    m_viewport = target.GetResolution();

    auto global_uniforms = GlobalUniforms {
        .screen_projection_matrix = camera.get_screen_projection_matrix(),
        .view_projection_matrix = camera.get_view_projection_matrix(),
        .nonscale_view_projection_matrix = camera.get_nonscale_view_projection_matrix(),
        .nonscale_projection_matrix = camera.get_nonscale_projection_matrix(),
        .inv_view_proj_matrix = camera.get_inv_view_projection_matrix(),
        .camera_position = camera.position(),
        .window_size = camera.viewport()
    };

    m_command_buffer->Begin();
    m_command_buffer->UpdateBuffer(*m_constant_buffer, 0, &global_uniforms, sizeof(global_uniforms));

    m_command_buffer->BeginRenderPass(target);
    m_command_buffer->SetViewport(m_viewport);
}

void Renderer::End() {
    ZoneScoped;

    m_command_buffer->End();
    m_command_queue->Submit(*m_command_buffer);
}

void Renderer::DestroyBatch(sge::Batch& batch) {
    const SpriteBatchPipeline& sprite_pipeline = batch.SpritePipeline();
    const auto glyph_pipeline = batch.GlyphPipeline();
    const auto line_pipeline = batch.LinePipeline();
    const auto shape_pipeline = batch.ShapePipeline();

    if (sprite_pipeline.additive)
        m_context->DeletePipeline(sprite_pipeline.additive.value());
    if (sprite_pipeline.alpha_blend)
        m_context->DeletePipeline(sprite_pipeline.alpha_blend.value());
    if (sprite_pipeline.premultiplied_alpha)
        m_context->DeletePipeline(sprite_pipeline.premultiplied_alpha.value());
    if (sprite_pipeline.opaque)
        m_context->DeletePipeline(sprite_pipeline.opaque.value());
    if (sprite_pipeline.depth_additive)
        m_context->DeletePipeline(sprite_pipeline.depth_additive.value());
    if (sprite_pipeline.depth_alpha_blend)
        m_context->DeletePipeline(sprite_pipeline.depth_alpha_blend.value());
    if (sprite_pipeline.depth_premultiplied_alpha)
        m_context->DeletePipeline(sprite_pipeline.depth_premultiplied_alpha.value());
    if (sprite_pipeline.depth_opaque)
        m_context->DeletePipeline(sprite_pipeline.depth_opaque.value());
    if (glyph_pipeline)
        m_context->DeletePipeline(glyph_pipeline.value());
    if (line_pipeline)
        m_context->DeletePipeline(line_pipeline.value());
    if (shape_pipeline)
        m_context->DeletePipeline(shape_pipeline.value());
}

static SGE_FORCE_INLINE uint32_t GetPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return pipeline.alpha_blend.value();
    case BlendMode::Additive: return pipeline.additive.value();
    case BlendMode::Opaque: return pipeline.opaque.value();
    case BlendMode::PremultipliedAlpha: return pipeline.premultiplied_alpha.value();
    default: SGE_UNREACHABLE();
    }
}

static SGE_FORCE_INLINE uint32_t GetDepthPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return pipeline.depth_alpha_blend.value();
    case BlendMode::Additive: return pipeline.depth_additive.value();
    case BlendMode::Opaque: return pipeline.depth_opaque.value();
    case BlendMode::PremultipliedAlpha: return pipeline.depth_premultiplied_alpha.value();
    default: SGE_UNREACHABLE();
    }
}

void Renderer::ApplyBatchDrawCommands(sge::Batch& batch) {
    ZoneScoped;

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    auto* const commands = m_command_buffer.get();

    int prev_flush_data_type = -1;
    int prev_texture_id = -1;

    sge::BlendMode prev_blend_mode = flush_queue[0].blend_mode;
    
    uint32_t sprite_pipeline = batch.DepthEnabled()
        ? GetDepthPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline())
        : GetPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline());

    size_t offset = 0;

    for (FlushData& flush_data : flush_queue) {
        if (prev_blend_mode != flush_data.blend_mode) {
            sprite_pipeline = batch.DepthEnabled()
                ? GetDepthPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline())
                : GetPipelineByBlendMode(prev_blend_mode, batch.SpritePipeline());
        }

        if (prev_flush_data_type != static_cast<int>(flush_data.type) || prev_blend_mode != flush_data.blend_mode) {
            switch (flush_data.type) {
            case FlushDataType::Sprite:
                commands->SetVertexBufferArray(m_sprite_batch_data.GetBufferArray());
                commands->SetPipelineState(m_context->GetOrCreatePipeline(sprite_pipeline));
                offset = batch.sprite_data().offset;
            break;

            case FlushDataType::Glyph:
                commands->SetVertexBufferArray(m_glyph_batch_data.GetBufferArray());
                commands->SetPipelineState(m_context->GetOrCreatePipeline(batch.GlyphPipeline().value()));
                offset = batch.glyph_data().offset;
            break;

            case FlushDataType::NinePatch:
                commands->SetVertexBufferArray(m_ninepatch_batch_data.GetBufferArray());
                commands->SetPipelineState(m_context->GetOrCreatePipeline(batch.NinepatchPipeline().value()));
                offset = batch.ninepatch_data().offset;
            break;

            case FlushDataType::Shape:
                commands->SetVertexBufferArray(m_shape_batch_data.GetBufferArray());
                commands->SetPipelineState(m_context->GetOrCreatePipeline(batch.ShapePipeline().value()));
                offset = batch.shape_data().offset;
            break;
            case FlushDataType::Line:
                commands->SetVertexBufferArray(m_line_batch_data.GetBufferArray());
                commands->SetPipelineState(m_context->GetOrCreatePipeline(batch.LinePipeline().value()));
                offset = batch.line_data().offset;
            break;
            }

            commands->SetResource(0, *m_constant_buffer);
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

        if (flush_data.texture.has_value() && prev_texture_id != flush_data.texture->id()) {
            const Texture& texture = flush_data.texture.value();
            SGE_ASSERT(texture.is_valid());

            commands->SetResource(1, texture);
            commands->SetResource(2, texture.sampler());

            prev_texture_id = texture.id();
        }

        commands->DrawInstanced(4, 0, flush_data.count, offset + flush_data.offset);

        prev_flush_data_type = static_cast<int>(flush_data.type);
        prev_blend_mode = flush_data.blend_mode;
    }

    flush_queue.clear();
}

void Renderer::SortBatchDrawCommands(sge::Batch& batch) {
    ZoneScoped;

    sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    std::sort(
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

            const Texture* a_texture = a.texture();
            const Texture* b_texture = b.texture();

            if (a_texture != nullptr && b_texture != nullptr) {
                if (a_texture->id() < b_texture->id()) return true;
                if (a_texture->id() > b_texture->id()) return false;
            }

            uint8_t a_bm = static_cast<uint8_t>(a.blend_mode());
            uint8_t b_bm = static_cast<uint8_t>(b.blend_mode());

            if (a_bm < b_bm) return true;
            if (a_bm > b_bm) return false;

            if (a.id() < b.id()) return true;
            if (a.id() > b.id()) return false;

            return false;
        }
    );
}

void Renderer::UpdateBatchBuffers(
    sge::Batch& batch,
    size_t begin
) {
    ZoneScoped;

    const sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    if (draw_commands.empty()) return;

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    Texture sprite_prev_texture;
    BlendMode sprite_prev_blend_mode;
    uint32_t sprite_count = 0;
    uint32_t sprite_total_count = 0;
    uint32_t sprite_vertex_offset = 0;
    uint32_t sprite_remaining = batch.sprite_data().count;

    Texture glyph_prev_texture;
    uint32_t glyph_count = 0;
    uint32_t glyph_total_count = 0;
    uint32_t glyph_vertex_offset = 0;
    uint32_t glyph_remaining = batch.glyph_data().count;

    Texture ninepatch_prev_texture;
    uint32_t ninepatch_count = 0;
    uint32_t ninepatch_total_count = 0;
    uint32_t ninepatch_vertex_offset = 0;
    uint32_t ninepatch_remaining = batch.ninepatch_data().count;

    uint32_t shape_count = 0;
    uint32_t shape_total_count = 0;
    uint32_t shape_vertex_offset = 0;
    uint32_t shape_remaining = batch.shape_data().count;

    uint32_t line_count = 0;
    uint32_t line_total_count = 0;
    uint32_t line_vertex_offset = 0;
    uint32_t line_remaining = batch.line_data().count;

    sge::IRect prev_scissor = draw_commands[begin].scissor();
    uint32_t prev_order = draw_commands[begin].order();

    size_t i = begin;
    for (; i < draw_commands.size(); ++i) {
        if (m_batch_instance_count >= MAX_QUADS) {
            break;
        }

        const DrawCommand& draw_command = draw_commands[i];

        if (prev_order != draw_command.order() || prev_scissor != draw_command.scissor()) {
            if (sprite_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .scissor = prev_scissor,
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = prev_order,
                    .type = FlushDataType::Sprite,
                    .blend_mode = sprite_prev_blend_mode
                });
                sprite_count = 0;
                sprite_vertex_offset = sprite_total_count;
            }

            if (glyph_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .scissor = prev_scissor,
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = prev_order,
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
                glyph_count = 0;
                glyph_vertex_offset = glyph_total_count;
            }

            if (ninepatch_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .scissor = prev_scissor,
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = prev_order,
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            if (shape_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = prev_scissor,
                    .offset = shape_vertex_offset,
                    .count = shape_count,
                    .order = prev_order,
                    .type = FlushDataType::Shape,
                    .blend_mode = draw_command.blend_mode()
                });
                shape_count = 0;
                shape_vertex_offset = shape_total_count;
            }

            if (line_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = prev_scissor,
                    .offset = line_vertex_offset,
                    .count = line_count,
                    .order = prev_order,
                    .type = FlushDataType::Line,
                    .blend_mode = draw_command.blend_mode()
                });
                line_count = 0;
                line_vertex_offset = line_total_count;
            }
        }

        switch (draw_command.type()) {
        case DrawCommand::DrawSprite: {
            if (sprite_remaining == 0) continue;

            const DrawCommandSprite& sprite_data = draw_command.sprite_data();

            if (sprite_total_count == 0) {
                sprite_prev_texture = sprite_data.texture;
                sprite_prev_blend_mode = draw_command.blend_mode();
            }

            const uint32_t prev_texture_id = sprite_prev_texture.id();
            const uint32_t curr_texture_id = sprite_data.texture.id();

            const sge::BlendMode curr_blend_mode = draw_command.blend_mode();

            const bool flush = (prev_texture_id != curr_texture_id || sprite_prev_blend_mode != curr_blend_mode);

            if (sprite_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Sprite,
                    .blend_mode = sprite_prev_blend_mode
                });
                sprite_count = 0;
                sprite_vertex_offset = sprite_total_count;
            }

            uint8_t flags = 0;
            flags |= sprite_data.ignore_camera_zoom << SpriteFlags::IgnoreCameraZoom;
            flags |= batch.IsUi() << SpriteFlags::UI;

            SpriteInstance* buffer = m_sprite_batch_data.GetBufferAndAdvance();
            buffer->position = sprite_data.position;
            buffer->rotation = sprite_data.rotation;
            buffer->size = sprite_data.size;
            buffer->offset = sprite_data.offset;
            buffer->uv_offset_scale = sprite_data.uv_offset_scale;
            buffer->color = sprite_data.color;
            buffer->outline_color = sprite_data.outline_color;
            buffer->outline_thickness = sprite_data.outline_thickness;
            buffer->flags = flags;

            ++sprite_count;
            ++sprite_total_count;
            --sprite_remaining;

            if (sprite_remaining == 0) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_data.texture,
                    .scissor = draw_command.scissor(),
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Sprite,
                    .blend_mode = draw_command.blend_mode()
                });
                sprite_count = 0;
                sprite_vertex_offset = sprite_total_count;
            }

            sprite_prev_texture = sprite_data.texture;
            sprite_prev_blend_mode = draw_command.blend_mode();
        } break;
        case DrawCommand::DrawGlyph: {
            if (glyph_remaining == 0) continue;

            const DrawCommandGlyph& glyph_data = draw_command.glyph_data();

            if (glyph_total_count == 0) {
                glyph_prev_texture = glyph_data.texture;
            }

            const bool flush = (glyph_prev_texture.id() != glyph_data.texture.id());

            if (glyph_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
                glyph_count = 0;
                glyph_vertex_offset = glyph_total_count;
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
            --glyph_remaining;

            if (glyph_remaining == 0) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_data.texture,
                    .scissor = draw_command.scissor(),
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
                glyph_count = 0;
                glyph_vertex_offset = glyph_total_count;
            }

            glyph_prev_texture = glyph_data.texture;
        } break;
        case DrawCommand::DrawNinePatch: {
            if (ninepatch_remaining == 0) continue;

            const DrawCommandNinePatch& ninepatch_data = draw_command.ninepatch_data();

            if (ninepatch_total_count == 0) {
                ninepatch_prev_texture = ninepatch_data.texture;
            }

            const uint32_t prev_texture_id = ninepatch_prev_texture.id();
            const uint32_t curr_texture_id = ninepatch_data.texture.id();

            const bool flush = (prev_texture_id != curr_texture_id);

            if (ninepatch_count > 0 && flush) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.IsUi() << SpriteFlags::UI;

            NinePatchInstance* buffer = m_ninepatch_batch_data.GetBufferAndAdvance();
            buffer->position = ninepatch_data.position;
            buffer->rotation = ninepatch_data.rotation;
            buffer->margin = ninepatch_data.margin;
            buffer->size = ninepatch_data.size;
            buffer->offset = ninepatch_data.offset;
            buffer->source_size = ninepatch_data.source_size;
            buffer->output_size = ninepatch_data.output_size;
            buffer->uv_offset_scale = ninepatch_data.uv_offset_scale;
            buffer->color = ninepatch_data.color;
            buffer->flags = flags;

            ++ninepatch_count;
            ++ninepatch_total_count;
            --ninepatch_remaining;

            if (ninepatch_remaining == 0) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_data.texture,
                    .scissor = draw_command.scissor(),
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            ninepatch_prev_texture = ninepatch_data.texture;
        } break;
        case DrawCommand::DrawShape: {
            if (shape_remaining == 0) continue;

            const DrawCommandShape& shape_data = draw_command.shape_data();

            uint8_t flags = 0;
            flags |= batch.IsUi() << ShapeFlags::UI;

            ShapeInstance* buffer = m_shape_batch_data.GetBufferAndAdvance();
            buffer->position = glm::vec3(shape_data.position, 0.0f);
            buffer->size = shape_data.size;
            buffer->offset = shape_data.offset;
            buffer->color = shape_data.color.to_vec4();
            buffer->border_color = shape_data.border_color.to_vec4();
            buffer->border_thickness = shape_data.border_thickness;
            buffer->border_radius = shape_data.border_radius;
            buffer->shape = shape_data.shape;
            buffer->flags = flags;

            ++shape_count;
            ++shape_total_count;
            --shape_remaining;

            const sge::IRect current_scissor = draw_command.scissor();

            if (shape_remaining == 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = current_scissor,
                    .offset = shape_vertex_offset,
                    .count = shape_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Shape,
                    .blend_mode = draw_command.blend_mode()
                });
                shape_count = 0;
                shape_vertex_offset = shape_total_count;
            }
        } break;
        case DrawCommand::DrawLine: {
            if (line_remaining == 0) continue;

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
            --line_remaining;
            
            if (line_remaining == 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = draw_command.scissor(),
                    .offset = line_vertex_offset,
                    .count = line_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Line,
                    .blend_mode = draw_command.blend_mode()
                });
                line_count = 0;
                line_vertex_offset = line_total_count;
            }
        };
        }

        if (m_batch_instance_count + 1 >= MAX_QUADS) {
            if (sprite_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Sprite,
                    .blend_mode = draw_command.blend_mode()
                });
            }

            if (glyph_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Glyph,
                    .blend_mode = draw_command.blend_mode()
                });
            }

            if (ninepatch_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .scissor = draw_command.scissor(),
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
            }

            if (shape_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = draw_command.scissor(),
                    .offset = shape_vertex_offset,
                    .count = shape_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Shape,
                    .blend_mode = draw_command.blend_mode()
                });
            }

            if (line_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .scissor = draw_command.scissor(),
                    .offset = line_vertex_offset,
                    .count = line_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Line,
                    .blend_mode = draw_command.blend_mode()
                });
            }
        }

        prev_scissor = draw_command.scissor();
        prev_order = draw_command.order();

        ++m_batch_instance_count;
    }

    batch.set_draw_commands_done(i);

    batch.sprite_data().count = sprite_remaining;
    batch.glyph_data().count = glyph_remaining;
    batch.ninepatch_data().count = ninepatch_remaining;
    batch.shape_data().count = shape_remaining;
    batch.line_data().count = line_remaining;
}


void Renderer::PrepareBatch(sge::Batch& batch) {
    if (batch.draw_commands().empty()) return;

    batch.sprite_data().offset = m_sprite_batch_data.Count();
    batch.glyph_data().offset = m_glyph_batch_data.Count();
    batch.ninepatch_data().offset = m_ninepatch_batch_data.Count();
    batch.shape_data().offset = m_shape_batch_data.Count();
    batch.line_data().offset = m_line_batch_data.Count();

    SortBatchDrawCommands(batch);
    UpdateBatchBuffers(batch);
}

void Renderer::UploadBatchData() {
    if (m_sprite_batch_data.Count() > 0) {
        m_sprite_batch_data.Update(*m_command_buffer);
    }

    if (m_glyph_batch_data.Count() > 0) {
        m_glyph_batch_data.Update(*m_command_buffer);
    }

    if (m_ninepatch_batch_data.Count() > 0) {
        m_ninepatch_batch_data.Update(*m_command_buffer);
    }

    if (m_shape_batch_data.Count() > 0) {
        m_shape_batch_data.Update(*m_command_buffer);
    }

    if (m_line_batch_data.Count() > 0) {
        m_line_batch_data.Update(*m_command_buffer);
    }
}

void Renderer::RenderBatch(sge::Batch& batch) {
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

Renderer::~Renderer() {
    const auto& context = m_context->Context();

    SGE_RESOURCE_RELEASE(m_constant_buffer);
    SGE_RESOURCE_RELEASE(m_command_buffer);

    m_sprite_batch_data.Destroy(context);
    m_ninepatch_batch_data.Destroy(context);
    m_glyph_batch_data.Destroy(context);
    m_shape_batch_data.Destroy(context);
}
