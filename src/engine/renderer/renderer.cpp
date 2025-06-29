#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/log.hpp>
#include <SGE/utils/alloc.hpp>
#include <SGE/utils/io.hpp>
#include <SGE/assert.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/types/attributes.hpp>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <utility>
#include <filesystem>

#include <tracy/Tracy.hpp>

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

template <typename T>
std::vector<LLGL::VertexAttribute> BatchData<T>::Init(const sge::Renderer& renderer, uint32_t size, const sge::Attributes& vertex_attributes, const sge::Attributes& instance_attributes) {
    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    m_buffer = checked_alloc<T>(size);
    m_buffer_ptr = m_buffer;

    LLGL::VertexFormat vertex_format;
    vertex_format.attributes = vertex_attributes.ToLLGL(renderer.Backend());

    LLGL::VertexFormat instance_format;
    instance_format.attributes = instance_attributes.ToLLGL(renderer.Backend(), vertex_format.attributes.size());

    m_vertex_buffer = renderer.CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "SpriteBatch VertexBuffer");
    m_instance_buffer = renderer.CreateVertexBuffer(size * sizeof(T), instance_format, "SpriteBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { m_vertex_buffer, m_instance_buffer };
    m_buffer_array = renderer.Context()->CreateBufferArray(2, buffers);

    std::vector<LLGL::VertexAttribute> total_attributes = vertex_format.attributes;
    total_attributes.insert(total_attributes.end(), instance_format.attributes.begin(), instance_format.attributes.end());
    return total_attributes;
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

static LLGL::Shader* CreateShader(Renderer& renderer, ShaderType shader_type, const void* data, size_t length, const char* entry_point, std::vector<LLGL::VertexAttribute> vertex_attributes = {}) {
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

SpriteBatchData Renderer::InitSpriteBatchPipeline() {
    ZoneScopedN("RenderBatchSprite::init");

    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    Attributes vertex_attributes = Attributes({
        Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
    });

    Attributes instance_attributes = Attributes({
        Attribute::Instance(LLGL::Format::RGB32Float, "i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_rotation", "I_Rotation", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_uv_offset_scale", "I_UvOffsetScale", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_outline_color", "I_OutlineColor", 1),
        Attribute::Instance(LLGL::Format::R32Float, "i_outline_thickness", "I_OutlineThickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_flags", "I_Flags", 1),
    });

    SpriteBatchData batchData;
    std::vector<LLGL::VertexAttribute> total_attributes = batchData.Init(*this, MAX_QUADS, vertex_attributes, instance_attributes);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "u_texture", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Sampler(backend.IsOpenGL() ? 3 : 4, "u_sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const void* vsSource = nullptr;
    size_t vsSize = 0;
    const void* psSource = nullptr;
    size_t psSize = 0;

    if (backend.IsD3D11() || backend.IsD3D12()) {
        vsSource = D3D11_SPRITE;
        vsSize = sizeof(D3D11_SPRITE);
        psSource = D3D11_SPRITE;
        psSize = sizeof(D3D11_SPRITE);
    } else if (backend.IsMetal()) {
        vsSource = METAL_SPRITE;
        vsSize = sizeof(METAL_SPRITE);
        psSource = METAL_SPRITE;
        psSize = sizeof(METAL_SPRITE);
    } else if (backend.IsOpenGL()) {
        vsSource = GL_SPRITE_VERT;
        vsSize = sizeof(GL_SPRITE_VERT);
        psSource = GL_SPRITE_FRAG;
        psSize = sizeof(GL_SPRITE_FRAG);
    } else if (backend.IsVulkan()) {
        vsSource = VULKAN_SPRITE_VERT;
        vsSize = sizeof(VULKAN_SPRITE_VERT);
        psSource = VULKAN_SPRITE_FRAG;
        psSize = sizeof(VULKAN_SPRITE_FRAG);
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", total_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
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

    {
        std::tuple<sge::BlendMode, LLGL::PipelineState**, const char*, int> pipelines[4] = {
            {
                sge::BlendMode::AlphaBlend, &batchData.pipeline_alpha_blend, "SpriteBatchPipelineAlphaBlend", 0
            },
            {
                sge::BlendMode::Additive, &batchData.pipeline_additive, "SpriteBatchPipelineAdditive", 1
            },
            {
                sge::BlendMode::Opaque, &batchData.pipeline_opaque, "SpriteBatchPipelineOpaque", 2
            },
            {
                sge::BlendMode::PremultipliedAlpha, &batchData.pipeline_premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlpha", 3
            },
        };

        for (const auto& [blend_mode, pointer, name, index] : pipelines) {
            pipelineDesc.debugName = name;
            bool hasInitialCache = false;
            LLGL::PipelineCache* pipelineCache = ReadPipelineCache(pipelineDesc.debugName, hasInitialCache);

            pipelineDesc.blend = blend_modes[index];
            LLGL::PipelineState* pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);
            *pointer = pipeline;

            if (!hasInitialCache) {
                SavePipelineCache(pipelineDesc.debugName, pipelineCache);
            }

            if (const LLGL::Report* report = pipeline->GetReport()) {
                if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            }
        }
    }
    {
        std::tuple<sge::BlendMode, LLGL::PipelineState**, const char*, int> pipelines[4] = {
            {
                sge::BlendMode::AlphaBlend, &batchData.pipeline_depth_alpha_blend, "SpriteBatchPipelineAlphaBlendDepth", 0
            },
            {
                sge::BlendMode::Additive, &batchData.pipeline_depth_additive, "SpriteBatchPipelineAdditiveDepth", 1
            },
            {
                sge::BlendMode::Opaque, &batchData.pipeline_depth_opaque, "SpriteBatchPipelineOpaqueDepth", 2
            },
            {
                sge::BlendMode::PremultipliedAlpha, &batchData.pipeline_depth_premultiplied_alpha, "SpriteBatchPipelinePremultipliedAlphaDepth", 3
            },
        };

        LLGL::GraphicsPipelineDescriptor depthPipelineDesc = pipelineDesc;
        depthPipelineDesc.depth = LLGL::DepthDescriptor {
            .testEnabled = true,
            .writeEnabled = true,
            .compareOp = LLGL::CompareOp::GreaterEqual,
        };

        for (const auto& [blend_mode, pointer, name, index] : pipelines) {
            depthPipelineDesc.debugName = name;

            bool hasInitialCache = false;
            LLGL::PipelineCache* pipelineCache = ReadPipelineCache(name, hasInitialCache);

            LLGL::PipelineState* pipeline = context->CreatePipelineState(depthPipelineDesc, pipelineCache);
            *pointer = pipeline;

            if (!hasInitialCache) {
                SavePipelineCache(name, pipelineCache);
            }

            if (const LLGL::Report* report = pipeline->GetReport()) {
                if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
            }
        }
    }

    return batchData;
}

NinePatchBatchData Renderer::InitNinepatchBatchPipeline() {
    ZoneScopedN("RenderBatchNinePatch::init");

    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    Attributes vertex_attributes = Attributes({
        Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
    });

    Attributes instance_attributes = Attributes({
        Attribute::Instance(LLGL::Format::RG32Float, "i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_rotation", "I_Rotation", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_source_size", "I_SourceSize", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_output_size", "I_OutputSize", 1),
        Attribute::Instance(LLGL::Format::RGBA32UInt, "i_margin", "I_Margin", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_uv_offset_scale", "I_UvOffsetScale", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_flags", "I_Flags", 1),
    });

    NinePatchBatchData batchData;
    std::vector<LLGL::VertexAttribute> total_attributes = batchData.Init(*this, MAX_QUADS, vertex_attributes, instance_attributes);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "u_texture", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Sampler(backend.IsOpenGL() ? 3 : 4, "u_sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const void* vsSource = nullptr;
    size_t vsSize = 0;
    const void* psSource = nullptr;
    size_t psSize = 0;

    if (backend.IsD3D11() || backend.IsD3D12()) {
        vsSource = D3D11_NINEPATCH;
        vsSize = sizeof(D3D11_NINEPATCH);
        psSource = D3D11_NINEPATCH;
        psSize = sizeof(D3D11_NINEPATCH);
    } else if (backend.IsMetal()) {
        vsSource = METAL_NINEPATCH;
        vsSize = sizeof(METAL_NINEPATCH);
        psSource = METAL_NINEPATCH;
        psSize = sizeof(METAL_NINEPATCH);
    } else if (backend.IsOpenGL()) {
        vsSource = GL_NINEPATCH_VERT;
        vsSize = sizeof(GL_NINEPATCH_VERT);
        psSource = GL_NINEPATCH_FRAG;
        psSize = sizeof(GL_NINEPATCH_FRAG);
    } else if (backend.IsVulkan()) {
        vsSource = VULKAN_NINEPATCH_VERT;
        vsSize = sizeof(VULKAN_NINEPATCH_VERT);
        psSource = VULKAN_NINEPATCH_FRAG;
        psSize = sizeof(VULKAN_NINEPATCH_FRAG);
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "NinePatchBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", total_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
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

    batchData.pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (!hasInitialCache) {
        SavePipelineCache("NinePatchBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = batchData.pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return batchData;
}

GlyphBatchData Renderer::InitGlyphBatchPipeline() {
    ZoneScopedN("RenderBatchGlyph::init");

    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    Attributes vertex_attributes = Attributes({
        Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
    });

    Attributes instance_attributes = Attributes({
        Attribute::Instance(LLGL::Format::RGB32Float, "i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_tex_size", "I_TexSize", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_uv", "I_UV", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_flags", "I_Flags", 1),
    });

    GlyphBatchData batchData;
    std::vector<LLGL::VertexAttribute> total_attributes = batchData.Init(*this, MAX_QUADS, vertex_attributes, instance_attributes);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Texture(3, "u_texture", LLGL::StageFlags::VertexStage),
        BindingLayoutItem::Sampler(backend.IsOpenGL() ? 3 : 4, "u_sampler", LLGL::StageFlags::FragmentStage)
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const void* vsSource = nullptr;
    size_t vsSize = 0;
    const void* psSource = nullptr;
    size_t psSize = 0;

    if (backend.IsD3D11() || backend.IsD3D12()) {
        vsSource = D3D11_FONT;
        vsSize = sizeof(D3D11_FONT);
        psSource = D3D11_FONT;
        psSize = sizeof(D3D11_FONT);
    } else if (backend.IsMetal()) {
        vsSource = METAL_FONT;
        vsSize = sizeof(METAL_FONT);
        psSource = METAL_FONT;
        psSize = sizeof(METAL_FONT);
    } else if (backend.IsOpenGL()) {
        vsSource = GL_FONT_VERT;
        vsSize = sizeof(GL_FONT_VERT);
        psSource = GL_FONT_FRAG;
        psSize = sizeof(GL_FONT_FRAG);
    } else if (backend.IsVulkan()) {
        vsSource = VULKAN_FONT_VERT;
        vsSize = sizeof(VULKAN_FONT_VERT);
        psSource = VULKAN_FONT_FRAG;
        psSize = sizeof(VULKAN_FONT_FRAG);
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "GlyphBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", total_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
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
    LLGL::PipelineCache* pipelineCache = ReadPipelineCache("GlyphBatchPipeline", hasInitialCache);

    batchData.pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (!hasInitialCache) {
        SavePipelineCache("GlyphBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = batchData.pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return batchData;
}

ShapeBatchData Renderer::InitShapeBatchPipeline() {
    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    Attributes vertex_attributes = Attributes({
        Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
    });

    Attributes instance_attributes = Attributes({
        Attribute::Instance(LLGL::Format::RGB32Float, "i_position", "I_Position", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_size", "I_Size", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_offset", "I_Offset", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_border_color", "I_BorderColor", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_border_radius", "I_BorderRadius", 1),
        Attribute::Instance(LLGL::Format::R32Float, "i_border_thickness", "I_BorderThickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_shape", "I_Shape", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_flags", "I_Flags", 1)
    });

    ShapeBatchData batchData;
    std::vector<LLGL::VertexAttribute> total_attributes = batchData.Init(*this, MAX_QUADS, vertex_attributes, instance_attributes);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const void* vsSource = nullptr;
    size_t vsSize = 0;
    const void* psSource = nullptr;
    size_t psSize = 0;

    if (backend.IsD3D11() || backend.IsD3D12()) {
        vsSource = D3D11_SHAPE;
        vsSize = sizeof(D3D11_SHAPE);
        psSource = D3D11_SHAPE;
        psSize = sizeof(D3D11_SHAPE);
    } else if (backend.IsMetal()) {
        vsSource = METAL_SHAPE;
        vsSize = sizeof(METAL_SHAPE);
        psSource = METAL_SHAPE;
        psSize = sizeof(METAL_SHAPE);
    } else if (backend.IsOpenGL()) {
        vsSource = GL_SHAPE_VERT;
        vsSize = sizeof(GL_SHAPE_VERT);
        psSource = GL_SHAPE_FRAG;
        psSize = sizeof(GL_SHAPE_FRAG);
    } else if (backend.IsVulkan()) {
        vsSource = VULKAN_SHAPE_VERT;
        vsSize = sizeof(VULKAN_SHAPE_VERT);
        psSource = VULKAN_SHAPE_FRAG;
        psSize = sizeof(VULKAN_SHAPE_FRAG);
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "ShapeBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", total_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
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

    batchData.pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (!hasInitialCache) {
        SavePipelineCache("ShapeBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = batchData.pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return batchData;
}

LineBatchData Renderer::InitLineBatchPipeline() {
    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    Attributes vertex_attributes = Attributes({
        Attribute::Vertex(LLGL::Format::RG32Float, "a_position", "Position"),
    });

    Attributes instance_attributes = Attributes({
        Attribute::Instance(LLGL::Format::RG32Float, "i_start", "I_Start", 1),
        Attribute::Instance(LLGL::Format::RG32Float, "i_end", "I_End", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_color", "I_Color", 1),
        Attribute::Instance(LLGL::Format::RGBA32Float, "i_border_radius", "I_Border_Radius", 1),
        Attribute::Instance(LLGL::Format::R32Float, "i_thickness", "I_Thickness", 1),
        Attribute::Instance(LLGL::Format::R8UInt, "i_flags", "I_Flags", 1),
    });

    LineBatchData batchData;
    std::vector<LLGL::VertexAttribute> total_attributes = batchData.Init(*this, MAX_QUADS, vertex_attributes, instance_attributes);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = BindingLayout({
        BindingLayoutItem::ConstantBuffer(2, "GlobalUniformBuffer", LLGL::StageFlags::VertexStage),
    });

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    const void* vsSource = nullptr;
    size_t vsSize = 0;
    const void* psSource = nullptr;
    size_t psSize = 0;

    if (backend.IsD3D11() || backend.IsD3D12()) {
        vsSource = D3D11_LINE;
        vsSize = sizeof(D3D11_LINE);
        psSource = D3D11_LINE;
        psSize = sizeof(D3D11_LINE);
    } else if (backend.IsMetal()) {
        vsSource = METAL_LINE;
        vsSize = sizeof(METAL_LINE);
        psSource = METAL_LINE;
        psSize = sizeof(METAL_LINE);
    } else if (backend.IsOpenGL()) {
        vsSource = GL_LINE_VERT;
        vsSize = sizeof(GL_LINE_VERT);
        psSource = GL_LINE_FRAG;
        psSize = sizeof(GL_LINE_FRAG);
    } else if (backend.IsVulkan()) {
        vsSource = VULKAN_LINE_VERT;
        vsSize = sizeof(VULKAN_LINE_VERT);
        psSource = VULKAN_LINE_FRAG;
        psSize = sizeof(VULKAN_LINE_FRAG);
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "LineBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", total_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
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

    batchData.pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

    if (!hasInitialCache) {
        SavePipelineCache("LineBatchPipeline", pipelineCache);
    }

    if (const LLGL::Report* report = batchData.pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("{}", report->GetText());
    }

    return batchData;
}

bool Renderer::InitEngine(RenderBackend backend, bool cache_pipelines, const std::string& cache_dir_path) {
    ZoneScopedN("Renderer::InitEngine");

    LLGL::Report report;

    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName = backend.ToString();

    if (backend.IsOpenGL()) {
        LLGL::RendererConfigurationOpenGL* config = new LLGL::RendererConfigurationOpenGL();
        config->majorVersion = 4;
        config->minorVersion = 3;
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

bool Renderer::Init(GLFWwindow* window, const LLGL::Extent2D& resolution, const WindowSettings& settings) {
    ZoneScopedN("Renderer::Init");

    const LLGL::RenderSystemPtr& context = m_context;

    m_surface = std::make_shared<CustomSurface>(window, resolution);

    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = settings.fullscreen;
    swapChainDesc.samples = settings.samples;

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

    m_sprite_batch_data = InitSpriteBatchPipeline();
    m_ninepatch_batch_data = InitNinepatchBatchPipeline();
    m_glyph_batch_data = InitGlyphBatchPipeline();
    m_shape_batch_data = InitShapeBatchPipeline();
    m_line_batch_data = InitLineBatchPipeline();

    ResizeBuffers(resolution);

    return true;
}

void Renderer::ResizeBuffers(LLGL::Extent2D size) {
    m_command_queue->WaitIdle();
    m_swap_chain->ResizeBuffers(size);
}

void Renderer::Begin(const Camera& camera) {
    ZoneScopedN("Renderer::Begin");

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
    ZoneScopedN("Renderer::End");

    m_command_buffer->End();
    m_command_queue->Submit(*m_command_buffer);

    m_swap_chain->Present();
}

static SGE_FORCE_INLINE LLGL::PipelineState* GetPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchData& data) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return data.pipeline_alpha_blend;
    case BlendMode::Additive: return data.pipeline_additive;
    case BlendMode::Opaque: return data.pipeline_opaque;
    case BlendMode::PremultipliedAlpha: return data.pipeline_premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

static SGE_FORCE_INLINE LLGL::PipelineState* GetDepthPipelineByBlendMode(sge::BlendMode blend_mode, const SpriteBatchData& data) {
    switch (blend_mode) {
    case BlendMode::AlphaBlend: return data.pipeline_depth_alpha_blend;
    case BlendMode::Additive: return data.pipeline_depth_additive;
    case BlendMode::Opaque: return data.pipeline_depth_opaque;
    case BlendMode::PremultipliedAlpha: return data.pipeline_depth_premultiplied_alpha;
    default: SGE_UNREACHABLE();
    }
}

void Renderer::ApplyBatchDrawCommands(sge::Batch& batch) {
    ZoneScopedN("Renderer::ApplyBatchDrawCommands");

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    auto* const commands = m_command_buffer;

    int prev_flush_data_type = -1;
    int prev_texture_id = -1;

    LLGL::PipelineState* sprite_pipeline = nullptr;

    sge::BlendMode prev_blend_mode = flush_queue[0].blend_mode;

    if (batch.DepthEnabled()) {
        sprite_pipeline = GetDepthPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data);
    } else {
        sprite_pipeline = GetPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data);
    }

    size_t offset = 0;

    for (FlushData& flush_data : flush_queue) {
        if (prev_blend_mode != flush_data.blend_mode) {
            if (batch.DepthEnabled()) {
                sprite_pipeline = GetDepthPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data);
            } else {
                sprite_pipeline = GetPipelineByBlendMode(prev_blend_mode, m_sprite_batch_data);
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
                commands->SetPipelineState(*m_glyph_batch_data.pipeline);
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
    ZoneScopedN("Renderer::SortBatchDrawCommands");

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
    ZoneScopedN("Renderer::UpdateBatchBuffers");

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
    ZoneScopedN("Renderer::RenderBatch");

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
    ZoneScopedN("Renderer::CreateSampler");
    LLGL::Sampler* sampler = m_context->CreateSampler(descriptor);
    return Sampler(sampler, descriptor);
}

Texture Renderer::CreateTexture(LLGL::TextureType type, LLGL::ImageFormat image_format, LLGL::DataType data_type, uint32_t width, uint32_t height, uint32_t layers, const Sampler& sampler, const void* data, bool generate_mip_maps) {
    ZoneScopedN("Renderer::CreateTexture");

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
    ZoneScopedN("Renderer::LoadShader");

    const RenderBackend backend = m_backend;
    const ShaderType shader_type = shader_path.shader_type;

    const std::string path = backend.AssetFolder() + shader_path.name + shader_type.FileExtension(backend);

    if (!FileExists(path.c_str())) {
        SGE_LOG_ERROR("Failed to find shader '{}'", path.c_str());
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
        shader_desc.source = path.c_str();
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
                SGE_LOG_ERROR("Failed to create a shader. File: {}\nError: {}", path.c_str(), report->GetText());
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
