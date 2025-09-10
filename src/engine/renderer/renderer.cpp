#include <cstddef>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>
#include <filesystem>

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
void BatchData<T>::Init(const sge::Renderer& renderer, uint32_t size, const LLGL::VertexFormat& vertex_format, const LLGL::VertexFormat& instance_format) {
    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    m_buffer = checked_alloc<T>(size);
    m_buffer_ptr = m_buffer;

    m_vertex_buffer = renderer.CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "SpriteBatch VertexBuffer");
    m_instance_buffer = renderer.CreateVertexBuffer(size * sizeof(T), instance_format, "SpriteBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer, m_instance_buffer };
    m_buffer_array = renderer.Context()->CreateBufferArray(2, buffers);
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

static LLGL::Shader* CreateShader(const Renderer& renderer, ShaderType shader_type, const void* data, size_t length, const char* entry_point, std::vector<LLGL::VertexAttribute> vertex_attributes = {}) {
    const RenderBackend backend = renderer.Backend();

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

    LLGL::Shader* shader = renderer.Context()->CreateShader(shader_desc);
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

LLGL::PipelineCache* Renderer::ReadPipelineCache(const std::string& name, bool& hasInitialCache) {
    // Try to read PSO cache from file
    const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
    const fs::path cachePath = fs::path(m_cache_pipeline_dir) / cacheFilename;

    LLGL::Blob pipelineCacheBlob = LLGL::Blob::CreateFromFile(cachePath.string());
    if (pipelineCacheBlob) {
        hasInitialCache = true;
    }

    return m_context->CreatePipelineCache(pipelineCacheBlob);
}

void Renderer::SavePipelineCache(const std::string& name, LLGL::PipelineCache* pipelineCache) {
    if (LLGL::Blob psoCache = pipelineCache->GetBlob())
    {
        const std::string cacheFilename = name + '.' + std::string(m_backend.ToString()) + ".cache";
        const fs::path cachePath = fs::path(m_cache_pipeline_dir) / cacheFilename;

        // Store PSO cache to file
        std::ofstream file{ cachePath, std::ios::out | std::ios::binary };
        file.write(
            reinterpret_cast<const char*>(psoCache.GetData()),
            static_cast<std::streamsize>(psoCache.GetSize())
        );
    }
}

SpriteBatchPipeline Renderer::CreateSpriteBatchPipeline(LLGL::Shader* fragment_shader) {
    static uint32_t count = 0;

    auto& context = Context();

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.vertexShader = m_sprite_vertex_shader;
    pipelineDesc.fragmentShader = fragment_shader;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = SwapChain()->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.rasterizer.multiSampleEnabled = m_swap_chain->GetSamples() > 1;

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
        std::tuple<sge::BlendMode, LLGL::PipelineState**, const char*, int> pipelines[4] = {
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
            pipelineDesc.debugName = name.c_str();
            
            bool hasInitialCache = false;
            LLGL::PipelineCache* pipelineCache = ReadPipelineCache(name, hasInitialCache);

            pipelineDesc.blend = blend_modes[index];
            LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);
            *pointer = pipeline;

            if (!hasInitialCache) {
                SavePipelineCache(name, pipelineCache);
            }

            if (const LLGL::Report* report = pipeline->GetReport()) {
                if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            }
        }
    }
    {
        std::tuple<sge::BlendMode, LLGL::PipelineState**, const char*, int> pipelines[4] = {
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

        LLGL::GraphicsPipelineDescriptor depthPipelineDesc = pipelineDesc;
        depthPipelineDesc.depth = LLGL::DepthDescriptor {
            .testEnabled = true,
            .writeEnabled = true,
            .compareOp = LLGL::CompareOp::GreaterEqual,
        };

        for (const auto& [blend_mode, pointer, pipelineName, index] : pipelines) {
            const std::string name = std::format("{}_{}", pipelineName, count);
            depthPipelineDesc.debugName = name.c_str();

            bool hasInitialCache = false;
            LLGL::PipelineCache* pipelineCache = ReadPipelineCache(name, hasInitialCache);

            LLGL::PipelineState* pipeline = context->CreatePipelineState(depthPipelineDesc, pipelineCache);
            *pointer = pipeline;

            if (m_cache_pipelines && !hasInitialCache) {
                SavePipelineCache(name, pipelineCache);
            }

            if (const LLGL::Report* report = pipeline->GetReport()) {
                if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            }
        }
    }

    ++count;

    return pipeline;
}

LLGL::PipelineState* Renderer::CreateNinepatchBatchPipeline() {
    auto& context = m_context;
    const RenderBackend backend = m_backend;

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "Texture", LLGL::StageFlags::FragmentStage),
        BindingLayoutItem::Sampler(4, "Sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetNinepatchShaderSourceCode(backend);
    LLGL::Shader* fragment_shader = CreateShader(*this, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "NinePatchBatch Pipeline";
    pipelineDesc.vertexShader = m_ninepatch_vertex_shader;
    pipelineDesc.fragmentShader = fragment_shader;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.rasterizer.multiSampleEnabled = m_swap_chain->GetSamples() > 1;
    pipelineDesc.blend = LLGL::BlendDescriptor {
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

    bool hasInitialCache = false;
    LLGL::PipelineCache* pipelineCache = ReadPipelineCache("NinePatchBatchPipeline", hasInitialCache);

    LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (m_cache_pipelines && !hasInitialCache) {
        SavePipelineCache("NinePatchBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return pipeline;
}

LLGL::PipelineState* Renderer::CreateGlyphBatchPipeline(LLGL::Shader* fragment_shader) {
    static uint32_t count = 0;

    auto& context = m_context;

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

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "GlyphBatch Pipeline";
    pipelineDesc.vertexShader = m_glyph_vertex_shader;
    pipelineDesc.fragmentShader = fragment_shader;
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.rasterizer.multiSampleEnabled = m_swap_chain->GetSamples() > 1;
    pipelineDesc.blend = LLGL::BlendDescriptor {
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

    const std::string name = std::format("GlyphBatchPipeline_{}", count);

    bool hasInitialCache = false;
    LLGL::PipelineCache* pipelineCache = ReadPipelineCache(name, hasInitialCache);

    LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (m_cache_pipelines && !hasInitialCache) {
        SavePipelineCache(name, pipelineCache);
    }

    if (const LLGL::Report* report = pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    ++count;

    return pipeline;
}

LLGL::PipelineState* Renderer::CreateShapeBatchPipeline() {
    auto& context = m_context;
    const RenderBackend backend = m_backend;

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetShapeShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(backend);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "ShapeBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, shader.vs_source, shader.vs_size, "VS", vertex_formats.total_attributes());
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.rasterizer.multiSampleEnabled = m_swap_chain->GetSamples() > 1;
    pipelineDesc.blend = LLGL::BlendDescriptor {
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

    bool hasInitialCache = false;
    LLGL::PipelineCache* pipelineCache = ReadPipelineCache("ShapeBatchPipeline", hasInitialCache);

    LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (m_cache_pipelines && !hasInitialCache) {
        SavePipelineCache("ShapeBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return pipeline;
}

LLGL::PipelineState* Renderer::CreateLineBatchPipeline() {
    auto& context = m_context;
    const RenderBackend backend = m_backend;

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer_std140", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    ShaderSourceCode shader = GetLineShaderSourceCode(backend);
    BatchVertexFormats vertex_formats = LineBatchVertexFormats(backend);

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "LineBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, shader.vs_source, shader.vs_size, "VS", vertex_formats.total_attributes());
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.rasterizer.multiSampleEnabled = m_swap_chain->GetSamples() > 1;
    pipelineDesc.blend = LLGL::BlendDescriptor {
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

    bool hasInitialCache = false;
    LLGL::PipelineCache* pipelineCache = ReadPipelineCache("LineBatchPipeline", hasInitialCache);

    LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (m_cache_pipelines && !hasInitialCache) {
        SavePipelineCache("LineBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return pipeline;
}


SpriteBatchData Renderer::InitSpriteBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = SpriteBatchVertexFormats(m_backend);

    ShaderSourceCode shader = GetSpriteShaderSourceCode(m_backend);
    LLGL::Shader* fragment_shader = CreateShader(*this, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");

    SpriteBatchData batchData;
    batchData.Init(*this, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    batchData.pipeline = CreateSpriteBatchPipeline(fragment_shader);
    return batchData;
}

NinePatchBatchData Renderer::InitNinepatchBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = NinepatchBatchVertexFormats(m_backend);

    NinePatchBatchData batchData;
    batchData.Init(*this, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    batchData.pipeline = CreateNinepatchBatchPipeline();
    return batchData;
}

GlyphBatchData Renderer::InitGlyphBatchData() {
    ZoneScoped;

    BatchVertexFormats vertex_formats = GlyphBatchVertexFormats(m_backend);

    ShaderSourceCode shader = GetFontShaderSourceCode(m_backend);
    LLGL::Shader* fragment_shader = CreateShader(*this, ShaderType::Fragment, shader.fs_source, shader.fs_size, "PS");

    GlyphBatchData batchData;
    batchData.Init(*this, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    batchData.pipeline = CreateGlyphBatchPipeline(fragment_shader);
    return batchData;
}

ShapeBatchData Renderer::InitShapeBatchData() {
    BatchVertexFormats vertex_formats = ShapeBatchVertexFormats(m_backend);

    ShapeBatchData batchData;
    batchData.Init(*this, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    batchData.pipeline = CreateShapeBatchPipeline();
    return batchData;
}

LineBatchData Renderer::InitLineBatchData() {
    BatchVertexFormats vertex_formats = LineBatchVertexFormats(m_backend);

    LineBatchData batchData;
    batchData.Init(*this, MAX_QUADS, vertex_formats.vertex, vertex_formats.instance);
    batchData.pipeline = CreateLineBatchPipeline();
    return batchData;
}

bool Renderer::InitEngine(RenderBackend backend, bool cache_pipelines, const std::string& cache_dir_path) {
    ZoneScoped;

    LLGL::Report report;

    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName = backend.ToString();

    if (backend.IsOpenGL()) {
        LLGL::RendererConfigurationOpenGL* config = new LLGL::RendererConfigurationOpenGL();
        config->contextProfile = LLGL::OpenGLContextProfile::CoreProfile;

        rendererDesc.rendererConfig = config;
        rendererDesc.rendererConfigSize = sizeof(LLGL::RendererConfigurationOpenGL);
    }

#if SGE_DEBUG
    m_debugger = new LLGL::RenderingDebugger();
    rendererDesc.flags    = LLGL::RenderSystemFlags::DebugDevice;
    rendererDesc.debugger = m_debugger;
#endif

    m_context = LLGL::RenderSystem::Load(rendererDesc, &report);
    m_backend = backend;

    if (backend.IsOpenGL()) {
        delete (LLGL::OpenGLContextProfile*) rendererDesc.rendererConfig;
    }

    if (report.HasErrors()) {
        SGE_LOG_ERROR("An error occured while loading render system: {}", report.GetText());
        return false;
    }

    if (m_context == nullptr) {
        SGE_LOG_ERROR("Couldn't load render system");
        return false;
    }

    m_cache_pipelines = cache_pipelines;
    m_cache_pipeline_dir = cache_dir_path;

    if (cache_pipelines && (!fs::is_directory(cache_dir_path) || !fs::exists(cache_dir_path))) {
        fs::create_directories(cache_dir_path);
    }

    return true;
}

static SGE_FORCE_INLINE LLGL::Shader* CreateBatchVertexShader(const Renderer& renderer, const ShaderSourceCode& source_code, const BatchVertexFormats& vertex_formats) {
    return CreateShader(renderer, ShaderType::Vertex, source_code.vs_source, source_code.vs_size, "VS", vertex_formats.total_attributes());
}

bool Renderer::Init(GLFWwindow* window, const LLGL::Extent2D& resolution, const WindowSettings& settings) {
    ZoneScoped;

    const LLGL::RenderSystemPtr& context = m_context;

    m_surface = std::make_shared<GlfwSurface>(window, resolution);

    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = settings.fullscreen;
    swapChainDesc.samples = settings.samples;
    swapChainDesc.colorBits = settings.transparent ? 32 : 24;

    m_swap_chain = context->CreateSwapChain(swapChainDesc, m_surface);
    m_swap_chain->SetVsyncInterval(settings.vsync ? 1 : 0);

    const LLGL::RendererInfo& info = GetRendererInfo();

    SGE_LOG_INFO("Renderer:             {}", info.rendererName.c_str());
    SGE_LOG_INFO("Device:               {}", info.deviceName.c_str());
    SGE_LOG_INFO("Vendor:               {}", info.vendorName.c_str());
    SGE_LOG_INFO("Shading Language:     {}", info.shadingLanguageName.c_str());

    SGE_LOG_INFO("Extensions:");
    for (const auto& extension : info.extensionNames) {
        SGE_LOG_INFO("  {}", extension.c_str());
    }

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = context->CreateCommandBuffer(command_buffer_desc);
    m_command_queue = context->GetCommandQueue();

    m_constant_buffer = CreateConstantBuffer(sizeof(ProjectionsUniform), "ConstantBuffer");

    m_sprite_vertex_shader = CreateBatchVertexShader(*this, GetSpriteShaderSourceCode(m_backend), SpriteBatchVertexFormats(m_backend));
    m_glyph_vertex_shader = CreateBatchVertexShader(*this, GetFontShaderSourceCode(m_backend), GlyphBatchVertexFormats(m_backend));
    m_ninepatch_vertex_shader = CreateBatchVertexShader(*this, GetNinepatchShaderSourceCode(m_backend), NinepatchBatchVertexFormats(m_backend));

    m_sprite_batch_data = InitSpriteBatchData();
    m_ninepatch_batch_data = InitNinepatchBatchData();
    m_glyph_batch_data = InitGlyphBatchData();
    m_shape_batch_data = InitShapeBatchData();
    m_line_batch_data = InitLineBatchData();

    ResizeBuffers(resolution);

    return true;
}

void Renderer::ResizeBuffers(LLGL::Extent2D size) {
    m_command_queue->WaitIdle();
    m_swap_chain->ResizeBuffers(size);
}

void Renderer::Begin(const Camera& camera) {
    ZoneScoped;

    m_viewport = camera.viewport();

    auto projections_uniform = ProjectionsUniform {
        .screen_projection_matrix = camera.get_screen_projection_matrix(),
        .view_projection_matrix = camera.get_view_projection_matrix(),
        .nonscale_view_projection_matrix = camera.get_nonscale_view_projection_matrix(),
        .nonscale_projection_matrix = camera.get_nonscale_projection_matrix(),
        .transform_matrix = camera.get_transform_matrix(),
        .inv_view_proj_matrix = camera.get_inv_view_projection_matrix(),
        .camera_position = camera.position(),
        .window_size = camera.viewport()
    };

    m_command_buffer->Begin();
    m_command_buffer->UpdateBuffer(*m_constant_buffer, 0, &projections_uniform, sizeof(projections_uniform));

    m_batch_instance_count = 0;

    m_sprite_batch_data.Reset();
    m_glyph_batch_data.Reset();
    m_ninepatch_batch_data.Reset();
    m_shape_batch_data.Reset();
    m_line_batch_data.Reset();
}

void Renderer::BeginPassWithViewport(LLGL::RenderTarget& target, const LLGL::Viewport& viewport) {
    m_command_buffer->BeginRenderPass(target);
    m_command_buffer->SetViewport(viewport);
}

void Renderer::End() {
    ZoneScoped;

    m_command_buffer->End();
    m_command_queue->Submit(*m_command_buffer);

    m_swap_chain->Present();
}

static SGE_FORCE_INLINE LLGL::PipelineState* GetPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return pipeline.alpha_blend;
    case BlendMode::Additive: return pipeline.additive;
    case BlendMode::Opaque: return pipeline.opaque;
    case BlendMode::PremultipliedAlpha: return pipeline.premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

static SGE_FORCE_INLINE LLGL::PipelineState* GetDepthPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchPipeline& pipeline) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return pipeline.depth_alpha_blend;
    case BlendMode::Additive: return pipeline.depth_additive;
    case BlendMode::Opaque: return pipeline.depth_opaque;
    case BlendMode::PremultipliedAlpha: return pipeline.depth_premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

void Renderer::ApplyBatchDrawCommands(sge::Batch& batch) {
    ZoneScoped;

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    auto* const commands = m_command_buffer;

    int prev_flush_data_type = -1;
    int prev_texture_id = -1;

    LLGL::PipelineState* sprite_pipeline = nullptr;

    sge::BlendMode prev_blend_mode = flush_queue[0].blend_mode;

    if (batch.DepthEnabled()) {
        sprite_pipeline = GetDepthPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data.pipeline);
    } else {
        sprite_pipeline = GetPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data.pipeline);
    }

    LLGL::PipelineState* glyph_pipeline = batch.glyph_pipeline() != nullptr
        ? batch.glyph_pipeline()
        : m_glyph_batch_data.pipeline;

    size_t offset = 0;

    for (FlushData& flush_data : flush_queue) {
        if (prev_blend_mode != flush_data.blend_mode) {
            if (batch.DepthEnabled()) {
                sprite_pipeline = GetDepthPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data.pipeline);
            } else {
                sprite_pipeline = GetPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data.pipeline);
            }
        }

        if (prev_flush_data_type != static_cast<int>(flush_data.type) || prev_blend_mode != flush_data.blend_mode) {
            switch (flush_data.type) {
            case FlushDataType::Sprite:
                commands->SetVertexBufferArray(*m_sprite_batch_data.GetBufferArray());
                commands->SetPipelineState(*sprite_pipeline);
                offset = batch.sprite_data().offset;
            break;

            case FlushDataType::Glyph:
                commands->SetVertexBufferArray(*m_glyph_batch_data.GetBufferArray());
                commands->SetPipelineState(*glyph_pipeline);
                offset = batch.glyph_data().offset;
            break;

            case FlushDataType::NinePatch:
                commands->SetVertexBufferArray(*m_ninepatch_batch_data.GetBufferArray());
                commands->SetPipelineState(*m_ninepatch_batch_data.pipeline);
                offset = batch.ninepatch_data().offset;
            break;

            case FlushDataType::Shape:
                commands->SetVertexBufferArray(*m_shape_batch_data.GetBufferArray());
                commands->SetPipelineState(*m_shape_batch_data.pipeline);
                offset = batch.shape_data().offset;
            break;
            case FlushDataType::Line:
                commands->SetVertexBufferArray(*m_line_batch_data.GetBufferArray());
                commands->SetPipelineState(*m_line_batch_data.pipeline);
                offset = batch.line_data().offset;
            break;
            }

            commands->SetResource(0, *m_constant_buffer);
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

            const Texture* a_texture = a.texture();
            const Texture* b_texture = b.texture();

            if (a_texture != nullptr && b_texture != nullptr) {
                return a_texture->id() < b.texture()->id();
            }

            uint8_t a_bm = static_cast<uint8_t>(a.blend_mode());
            uint8_t b_bm = static_cast<uint8_t>(b.blend_mode());

            if (a_bm < b_bm) return true;
            if (a_bm > b_bm) return false;

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

    size_t i = 0;
    for (i = begin; i < draw_commands.size(); ++i) {
        if (m_batch_instance_count >= MAX_QUADS) {
            break;
        }

        uint32_t next_order = draw_commands[i].order();

        if (i < draw_commands.size() - 1) {
            next_order = draw_commands[i + 1].order();
        }

        const DrawCommand& draw_command = draw_commands[i];

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

            const uint32_t current_order = draw_command.order();

            if (sprite_count > 0 && (prev_texture_id != curr_texture_id || sprite_prev_blend_mode != curr_blend_mode)) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = current_order,
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

            if (sprite_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_data.texture,
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

            const uint32_t current_order = draw_command.order();

            if (glyph_count > 0 && glyph_prev_texture.id() != glyph_data.texture.id()) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = current_order,
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

            if (glyph_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_data.texture,
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

            const uint32_t current_order = draw_command.order();

            if (ninepatch_count > 0 && prev_texture_id != curr_texture_id) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = current_order,
                    .type = FlushDataType::NinePatch,
                    .blend_mode = draw_command.blend_mode()
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.IsUi() << ShapeFlags::UI;

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

            if (ninepatch_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_data.texture,
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

            const uint32_t current_order = draw_command.order();

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

            if (shape_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
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

            const uint32_t current_order = draw_command.order();

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

            if (line_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
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
                    .offset = line_vertex_offset,
                    .count = line_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Line,
                    .blend_mode = draw_command.blend_mode()
                });
            }
        }

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
        m_sprite_batch_data.Update(m_command_buffer);
    }

    if (m_glyph_batch_data.Count() > 0) {
        m_glyph_batch_data.Update(m_command_buffer);
    }

    if (m_ninepatch_batch_data.Count() > 0) {
        m_ninepatch_batch_data.Update(m_command_buffer);
    }

    if (m_shape_batch_data.Count() > 0) {
        m_shape_batch_data.Update(m_command_buffer);
    }

    if (m_line_batch_data.Count() > 0) {
        m_line_batch_data.Update(m_command_buffer);
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

Sampler Renderer::CreateSampler(const LLGL::SamplerDescriptor& descriptor) {
    ZoneScoped;
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    return Sampler(sampler, descriptor);
}

Texture Renderer::CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const Sampler& sampler, const void* data, bool generate_mip_maps) {
    ZoneScoped;

    LLGL::TextureDescriptor texture_desc;
    texture_desc.type = type;
    texture_desc.extent = LLGL::Extent3D(width, height, 1);
    texture_desc.arrayLayers = layers;
    texture_desc.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment;
    texture_desc.cpuAccessFlags = 0;
    texture_desc.miscFlags = LLGL::MiscFlags::GenerateMips * generate_mip_maps;
    texture_desc.mipLevels = generate_mip_maps ? 0 : 1;

    uint32_t components = LLGL::ImageFormatSize(image_format);

    LLGL::ImageView image_view;
    image_view.format = image_format;
    image_view.dataType = data_type;
    image_view.data = data;
    image_view.dataSize = width * height * layers * components;

    uint32_t id = m_texture_index++;

    return Texture(id, sampler, glm::uvec2(width, height), m_context->CreateTexture(texture_desc, &image_view));
}

LLGL::Shader* Renderer::LoadShader(const ShaderPath& shader_path, const std::vector<ShaderDef>& shader_defs, const std::vector<LLGL::VertexAttribute>& vertex_attributes) {
    ZoneScoped;

    const RenderBackend backend = m_backend;
    const ShaderType shader_type = shader_path.shader_type;

    const std::string filename = backend.IsMetal()
        ? std::format("{}{}", shader_path.name, shader_type.FileExtension(backend))
        : shader_type.IsCompute()
            ? std::format("{}.{}.{}{}", shader_path.func_name, shader_path.name, shader_type.Stage(), shader_type.FileExtension(backend))
            : std::format("{}.{}{}", shader_path.name, shader_type.Stage(), shader_type.FileExtension(backend));

    const fs::path path = fs::path(backend.AssetFolder()) / filename;
    const std::string path_str = path.string().c_str();

    if (!fs::exists(path)) {
        SGE_LOG_ERROR("Failed to find shader '{}'", path_str.c_str());
        return nullptr;
    }

    std::string shader_source;

    if (!backend.IsVulkan()) {
        std::ifstream shader_file;
        shader_file.open(path);

        std::stringstream shader_source_str;
        shader_source_str << shader_file.rdbuf();

        shader_source = shader_source_str.str();

        for (const ShaderDef& shader_def : shader_defs) {
            size_t pos;
            while ((pos = shader_source.find(shader_def.name)) != std::string::npos) {
                shader_source.replace(pos, shader_def.name.length(), shader_def.value);
            }
        }

        shader_file.close();
    }

    LLGL::ShaderDescriptor shader_desc;
    shader_desc.type = shader_type.ToLLGLType();
    shader_desc.sourceType = LLGL::ShaderSourceType::CodeString;

    if (shader_type.IsVertex()) {
        shader_desc.vertex.inputAttribs = vertex_attributes;
    }

    if (backend.IsOpenGL() && shader_type.IsFragment()) {
        shader_desc.fragment.outputAttribs = {
            { "frag_color", LLGL::Format::RGBA8UNorm, 0, LLGL::SystemValue::Color }
        };
    }

    if (backend.IsVulkan()) {
        shader_desc.source = path_str.c_str();
        shader_desc.sourceType = LLGL::ShaderSourceType::BinaryFile;
    } else {
        shader_desc.entryPoint = shader_type.IsCompute() ? shader_path.func_name.c_str() : shader_type.EntryPoint(backend);
        shader_desc.source = shader_source.c_str();
        shader_desc.sourceSize = shader_source.length();
        shader_desc.profile = shader_type.Profile(backend);
    }

#if SGE_DEBUG
    shader_desc.flags |= LLGL::ShaderCompileFlags::NoOptimization;
#else
    shader_desc.flags |= LLGL::ShaderCompileFlags::OptimizationLevel3;
#endif

    LLGL::Shader* shader = m_context->CreateShader(shader_desc);
    if (const LLGL::Report* report = shader->GetReport()) {
        if (*report->GetText() != '\0') {
            if (report->HasErrors()) {
                SGE_LOG_ERROR("Failed to create a shader. File: {}\nError: {}", path_str.c_str(), report->GetText());
                return nullptr;
            }

            SGE_LOG_INFO("{}", report->GetText());
        }
    }

    return shader;
}

#if SGE_DEBUG
void Renderer::PrintDebugInfo() {
    LLGL::FrameProfile profile;
    m_debugger->FlushProfile(&profile);

    SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
}
#endif

void Renderer::Terminate() {
    const auto& context = m_context;

    SGE_RESOURCE_RELEASE(m_constant_buffer);
    SGE_RESOURCE_RELEASE(m_command_buffer);
    SGE_RESOURCE_RELEASE(m_swap_chain);

    m_sprite_batch_data.Destroy(context);
    m_ninepatch_batch_data.Destroy(context);
    m_glyph_batch_data.Destroy(context);
    m_shape_batch_data.Destroy(context);

    LLGL::RenderSystem::Unload(std::move(m_context));

#if SGE_DEBUG
    delete m_debugger;
#endif
}
