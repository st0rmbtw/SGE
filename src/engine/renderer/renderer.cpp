#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/log.hpp>
#include <SGE/utils.hpp>

#include <cstddef>
#include <fstream>
#include <optional>
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

#include "LLGL/PipelineCache.h"
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
                SGE_LOG_ERROR("Failed to create a shader: %s", report->GetText());
                return nullptr;
            }
            
            SGE_LOG_INFO("%s", report->GetText());
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

    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    LLGL::VertexFormat vertex_format;
    LLGL::VertexFormat instance_format;

    if (backend.IsGLSL()) {
        vertex_format.AppendAttribute({ "a_position", LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else if (backend.IsHLSL()) {
        vertex_format.AppendAttribute({ "Position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else {
        vertex_format.AppendAttribute({ "position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    }

    if (backend.IsGLSL()) {
        instance_format.attributes = {
            {"i_position",           LLGL::Format::RGB32Float,  1,  offsetof(SpriteInstance,position),          sizeof(SpriteInstance), 1, 1 },
            {"i_rotation",           LLGL::Format::RGBA32Float, 2,  offsetof(SpriteInstance,rotation),          sizeof(SpriteInstance), 1, 1 },
            {"i_size",               LLGL::Format::RG32Float,   3,  offsetof(SpriteInstance,size),              sizeof(SpriteInstance), 1, 1 },
            {"i_offset",             LLGL::Format::RG32Float,   4,  offsetof(SpriteInstance,offset),            sizeof(SpriteInstance), 1, 1 },
            {"i_uv_offset_scale",    LLGL::Format::RGBA32Float, 5,  offsetof(SpriteInstance,uv_offset_scale),   sizeof(SpriteInstance), 1, 1 },
            {"i_color",              LLGL::Format::RGBA32Float, 6,  offsetof(SpriteInstance,color),             sizeof(SpriteInstance), 1, 1 },
            {"i_outline_color",      LLGL::Format::RGBA32Float, 7,  offsetof(SpriteInstance,outline_color),     sizeof(SpriteInstance), 1, 1 },
            {"i_outline_thickness",  LLGL::Format::R32Float,    8,  offsetof(SpriteInstance,outline_thickness), sizeof(SpriteInstance), 1, 1 },
            {"i_flags",              LLGL::Format::R8UInt,      9,  offsetof(SpriteInstance,flags),             sizeof(SpriteInstance), 1, 1 },
        };
    } else if (backend.IsHLSL()) {
        instance_format.attributes = {
            {"I_Position",         LLGL::Format::RGB32Float,  1,  offsetof(SpriteInstance,position),          sizeof(SpriteInstance), 1, 1 },
            {"I_Rotation",         LLGL::Format::RGBA32Float, 2,  offsetof(SpriteInstance,rotation),          sizeof(SpriteInstance), 1, 1 },
            {"I_Size",             LLGL::Format::RG32Float,   3,  offsetof(SpriteInstance,size),              sizeof(SpriteInstance), 1, 1 },
            {"I_Offset",           LLGL::Format::RG32Float,   4,  offsetof(SpriteInstance,offset),            sizeof(SpriteInstance), 1, 1 },
            {"I_UvOffsetScale",    LLGL::Format::RGBA32Float, 5,  offsetof(SpriteInstance,uv_offset_scale),   sizeof(SpriteInstance), 1, 1 },
            {"I_Color",            LLGL::Format::RGBA32Float, 6,  offsetof(SpriteInstance,color),             sizeof(SpriteInstance), 1, 1 },
            {"I_OutlineColor",     LLGL::Format::RGBA32Float, 7,  offsetof(SpriteInstance,outline_color),     sizeof(SpriteInstance), 1, 1 },
            {"I_OutlineThickness", LLGL::Format::R32Float,    8,  offsetof(SpriteInstance,outline_thickness), sizeof(SpriteInstance), 1, 1 },
            {"I_Flags",            LLGL::Format::R8UInt,      9,  offsetof(SpriteInstance,flags),             sizeof(SpriteInstance), 1, 1 },
        };
    } else {
        instance_format.attributes = {
            {"i_position",           LLGL::Format::RGB32Float,  1,  offsetof(SpriteInstance,position),          sizeof(SpriteInstance), 1, 1 },
            {"i_rotation",           LLGL::Format::RGBA32Float, 2,  offsetof(SpriteInstance,rotation),          sizeof(SpriteInstance), 1, 1 },
            {"i_size",               LLGL::Format::RG32Float,   3,  offsetof(SpriteInstance,size),              sizeof(SpriteInstance), 1, 1 },
            {"i_offset",             LLGL::Format::RG32Float,   4,  offsetof(SpriteInstance,offset),            sizeof(SpriteInstance), 1, 1 },
            {"i_uv_offset_scale",    LLGL::Format::RGBA32Float, 5,  offsetof(SpriteInstance,uv_offset_scale),   sizeof(SpriteInstance), 1, 1 },
            {"i_color",              LLGL::Format::RGBA32Float, 6,  offsetof(SpriteInstance,color),             sizeof(SpriteInstance), 1, 1 },
            {"i_outline_color",      LLGL::Format::RGBA32Float, 7,  offsetof(SpriteInstance,outline_color),     sizeof(SpriteInstance), 1, 1 },
            {"i_outline_thickness",  LLGL::Format::R32Float,    8,  offsetof(SpriteInstance,outline_thickness), sizeof(SpriteInstance), 1, 1 },
            {"i_flags",              LLGL::Format::R8UInt,      9,  offsetof(SpriteInstance,flags),             sizeof(SpriteInstance), 1, 1 },
        };
    }

    SpriteBatchData batchData;

    batchData.buffer = checked_alloc<SpriteInstance>(MAX_QUADS);
    batchData.buffer_ptr = batchData.buffer;

    batchData.vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "SpriteBatch VertexBuffer");
    batchData.instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(SpriteInstance), instance_format, "SpriteBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { batchData.vertex_buffer, batchData.instance_buffer };
    batchData.buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
        LLGL::BindingDescriptor("u_texture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(3)),
        LLGL::BindingDescriptor("u_sampler", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(backend.IsOpenGL() ? 3 : 4)),
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    std::vector<LLGL::VertexAttribute> vertex_attributes = vertex_format.attributes;
    vertex_attributes.insert(vertex_attributes.end(), instance_format.attributes.begin(), instance_format.attributes.end());

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
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "SpriteBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", vertex_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = SwapChain()->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
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

    {
        bool hasInitialCache = false;
        LLGL::PipelineCache* pipelineCache = ReadPipelineCache("SpriteBatchPipeline", hasInitialCache);

        batchData.pipeline = context->CreatePipelineState(pipelineDesc, pipelineCache);

        if (!hasInitialCache) {
            SavePipelineCache("SpriteBatchPipeline", pipelineCache);
        }
    }
    {

        LLGL::GraphicsPipelineDescriptor depthPipelineDesc = pipelineDesc;
        depthPipelineDesc.debugName = "SpriteBatch Pipeline Depth";
        depthPipelineDesc.depth = LLGL::DepthDescriptor {
            .testEnabled = true,
            .writeEnabled = true,
            .compareOp = LLGL::CompareOp::GreaterEqual,
        };

        bool hasInitialCache = false;
        LLGL::PipelineCache* pipelineCache = ReadPipelineCache("SpriteBatchPipelineDepth", hasInitialCache);
        
        batchData.pipeline_depth = context->CreatePipelineState(depthPipelineDesc, pipelineCache);

        if (!hasInitialCache) {
            SavePipelineCache("SpriteBatchPipelineDepth", pipelineCache);
        }
    }

    if (const LLGL::Report* report = batchData.pipeline->GetReport()) {
        if (report->HasErrors()) SGE_LOG_ERROR("%s", report->GetText());
    }

    return batchData;
}

NinePatchBatchData Renderer::InitNinepatchBatchPipeline() {
    ZoneScopedN("RenderBatchNinePatch::init");

    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    LLGL::VertexFormat vertex_format;
    LLGL::VertexFormat instance_format;

    if (backend.IsGLSL()) {
        vertex_format.AppendAttribute({ "a_position", LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else if (backend.IsHLSL()) {
        vertex_format.AppendAttribute({ "Position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else {
        vertex_format.AppendAttribute({ "position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    }

    if (backend.IsGLSL()) {
        instance_format.attributes = {
            {"i_position",         LLGL::Format::RG32Float,   1,  offsetof(NinePatchInstance,position),          sizeof(NinePatchInstance), 1, 1 },
            {"i_rotation",         LLGL::Format::RGBA32Float, 2,  offsetof(NinePatchInstance,rotation),          sizeof(NinePatchInstance), 1, 1 },
            {"i_size",             LLGL::Format::RG32Float,   3,  offsetof(NinePatchInstance,size),              sizeof(NinePatchInstance), 1, 1 },
            {"i_offset",           LLGL::Format::RG32Float,   4,  offsetof(NinePatchInstance,offset),            sizeof(NinePatchInstance), 1, 1 },
            {"i_source_size",      LLGL::Format::RG32Float,   5,  offsetof(NinePatchInstance,source_size),       sizeof(NinePatchInstance), 1, 1 },
            {"i_output_size",      LLGL::Format::RG32Float,   6,  offsetof(NinePatchInstance,output_size),       sizeof(NinePatchInstance), 1, 1 },
            {"i_margin",           LLGL::Format::RGBA32UInt,  7,  offsetof(NinePatchInstance,margin),            sizeof(NinePatchInstance), 1, 1 },
            {"i_uv_offset_scale",  LLGL::Format::RGBA32Float, 8,  offsetof(NinePatchInstance,uv_offset_scale),   sizeof(NinePatchInstance), 1, 1 },
            {"i_color",            LLGL::Format::RGBA32Float, 9,  offsetof(NinePatchInstance,color),             sizeof(NinePatchInstance), 1, 1 },
            {"i_flags",            LLGL::Format::R8UInt,     10, offsetof(NinePatchInstance,flags),             sizeof(NinePatchInstance), 1, 1 },
        };
    } else if (backend.IsHLSL()) {
        instance_format.attributes = {
            {"I_Position",         LLGL::Format::RG32Float,   1,  offsetof(NinePatchInstance,position),          sizeof(NinePatchInstance), 1, 1 },
            {"I_Rotation",         LLGL::Format::RGBA32Float, 2,  offsetof(NinePatchInstance,rotation),          sizeof(NinePatchInstance), 1, 1 },
            {"I_Size",             LLGL::Format::RG32Float,   3,  offsetof(NinePatchInstance,size),              sizeof(NinePatchInstance), 1, 1 },
            {"I_Offset",           LLGL::Format::RG32Float,   4,  offsetof(NinePatchInstance,offset),            sizeof(NinePatchInstance), 1, 1 },
            {"I_SourceSize",       LLGL::Format::RG32Float,   5,  offsetof(NinePatchInstance,source_size),       sizeof(NinePatchInstance), 1, 1 },
            {"I_OutputSize",       LLGL::Format::RG32Float,   6,  offsetof(NinePatchInstance,output_size),       sizeof(NinePatchInstance), 1, 1 },
            {"I_Margin",           LLGL::Format::RGBA32UInt,  7,  offsetof(NinePatchInstance,margin),            sizeof(NinePatchInstance), 1, 1 },
            {"I_UvOffsetScale",    LLGL::Format::RGBA32Float, 8,  offsetof(NinePatchInstance,uv_offset_scale),   sizeof(NinePatchInstance), 1, 1 },
            {"I_Color",            LLGL::Format::RGBA32Float, 9,  offsetof(NinePatchInstance,color),             sizeof(NinePatchInstance), 1, 1 },
            {"I_Flags",            LLGL::Format::R8UInt,     10, offsetof(NinePatchInstance,flags),             sizeof(NinePatchInstance), 1, 1 },
        };
    } else {
        instance_format.attributes = {
            {"i_position",         LLGL::Format::RG32Float,   1,  offsetof(NinePatchInstance,position),          sizeof(NinePatchInstance), 1, 1 },
            {"i_rotation",         LLGL::Format::RGBA32Float, 2,  offsetof(NinePatchInstance,rotation),          sizeof(NinePatchInstance), 1, 1 },
            {"i_size",             LLGL::Format::RG32Float,   3,  offsetof(NinePatchInstance,size),              sizeof(NinePatchInstance), 1, 1 },
            {"i_offset",           LLGL::Format::RG32Float,   4,  offsetof(NinePatchInstance,offset),            sizeof(NinePatchInstance), 1, 1 },
            {"i_source_size",      LLGL::Format::RG32Float,   5,  offsetof(NinePatchInstance,source_size),       sizeof(NinePatchInstance), 1, 1 },
            {"i_output_size",      LLGL::Format::RG32Float,   6,  offsetof(NinePatchInstance,output_size),       sizeof(NinePatchInstance), 1, 1 },
            {"i_margin",           LLGL::Format::RGBA32UInt,  7,  offsetof(NinePatchInstance,margin),            sizeof(NinePatchInstance), 1, 1 },
            {"i_uv_offset_scale",  LLGL::Format::RGBA32Float, 8,  offsetof(NinePatchInstance,uv_offset_scale),   sizeof(NinePatchInstance), 1, 1 },
            {"i_color",            LLGL::Format::RGBA32Float, 9,  offsetof(NinePatchInstance,color),             sizeof(NinePatchInstance), 1, 1 },
            {"i_flags",            LLGL::Format::R8UInt,     10, offsetof(NinePatchInstance,flags),             sizeof(NinePatchInstance), 1, 1 },
        };
    }

    NinePatchBatchData batchData;

    batchData.buffer = checked_alloc<NinePatchInstance>(MAX_QUADS);
    batchData.buffer_ptr = batchData.buffer;

    batchData.vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "NinePatchBatch VertexBuffer");
    batchData.instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(NinePatchInstance), instance_format, "NinePatchBatch InstanceBuffer");

    {
        LLGL::Buffer* buffers[] = { batchData.vertex_buffer, batchData.instance_buffer };
        batchData.buffer_array = context->CreateBufferArray(2, buffers);
    }

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
        LLGL::BindingDescriptor("u_texture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(3)),
        LLGL::BindingDescriptor("u_sampler", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(backend.IsOpenGL() ? 3 : 4)),
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    std::vector<LLGL::VertexAttribute> vertex_attributes = vertex_format.attributes;
    vertex_attributes.insert(vertex_attributes.end(), instance_format.attributes.begin(), instance_format.attributes.end());

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
    } else if (backend.IsOpenGL()) {
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "NinePatchBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", vertex_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
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
        if (report->HasErrors()) SGE_LOG_ERROR("%s", report->GetText());
    }

    return batchData;
}

GlyphBatchData Renderer::InitGlyphBatchPipeline() {
    ZoneScopedN("RenderBatchGlyph::init");

    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    LLGL::VertexFormat vertex_format;
    LLGL::VertexFormat instance_format;

    if (backend.IsGLSL()) {
        vertex_format.AppendAttribute({"a_position", LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0});
    } else if (backend.IsHLSL()) {
        vertex_format.AppendAttribute({"Position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0});
    } else {
        vertex_format.AppendAttribute({"position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0});
    }

    if (backend.IsGLSL()) {
        instance_format.attributes = {
            {"i_color",     LLGL::Format::RGB32Float, 1, offsetof(GlyphInstance,color),    sizeof(GlyphInstance), 1, 1},
            {"i_position",  LLGL::Format::RGB32Float, 2, offsetof(GlyphInstance,pos),      sizeof(GlyphInstance), 1, 1},
            {"i_size",      LLGL::Format::RG32Float,  3, offsetof(GlyphInstance,size),     sizeof(GlyphInstance), 1, 1},
            {"i_tex_size",  LLGL::Format::RG32Float,  4, offsetof(GlyphInstance,tex_size), sizeof(GlyphInstance), 1, 1},
            {"i_uv",        LLGL::Format::RG32Float,  5, offsetof(GlyphInstance,uv),       sizeof(GlyphInstance), 1, 1},
            {"i_flags",     LLGL::Format::R8UInt,     6, offsetof(GlyphInstance,flags),    sizeof(GlyphInstance), 1, 1},
        };
    } else if (backend.IsHLSL()) {
        instance_format.attributes = {
            {"I_Color",    LLGL::Format::RGB32Float, 1, offsetof(GlyphInstance,color),    sizeof(GlyphInstance), 1, 1},
            {"I_Position", LLGL::Format::RG32Float,  2, offsetof(GlyphInstance,pos),      sizeof(GlyphInstance), 1, 1},
            {"I_Size",     LLGL::Format::RG32Float,  3, offsetof(GlyphInstance,size),     sizeof(GlyphInstance), 1, 1},
            {"I_TexSize",  LLGL::Format::RG32Float,  4, offsetof(GlyphInstance,tex_size), sizeof(GlyphInstance), 1, 1},
            {"I_UV",       LLGL::Format::RG32Float,  5, offsetof(GlyphInstance,uv),       sizeof(GlyphInstance), 1, 1},
            {"I_Flags",    LLGL::Format::R8UInt,     6, offsetof(GlyphInstance,flags),    sizeof(GlyphInstance), 1, 1},
        };
    } else {
        instance_format.attributes = {
            {"i_color",     LLGL::Format::RGB32Float, 1, offsetof(GlyphInstance,color),    sizeof(GlyphInstance), 1, 1},
            {"i_position",  LLGL::Format::RGB32Float, 2, offsetof(GlyphInstance,pos),      sizeof(GlyphInstance), 1, 1},
            {"i_size",      LLGL::Format::RG32Float,  3, offsetof(GlyphInstance,size),     sizeof(GlyphInstance), 1, 1},
            {"i_tex_size",  LLGL::Format::RG32Float,  4, offsetof(GlyphInstance,tex_size), sizeof(GlyphInstance), 1, 1},
            {"i_uv",        LLGL::Format::RG32Float,  5, offsetof(GlyphInstance,uv),       sizeof(GlyphInstance), 1, 1},
            {"i_flags",     LLGL::Format::R8UInt,     6, offsetof(GlyphInstance,flags),    sizeof(GlyphInstance), 1, 1},
        };
    }

    GlyphBatchData batchData;

    batchData.buffer = checked_alloc<GlyphInstance>(MAX_QUADS);
    batchData.buffer_ptr = batchData.buffer;

    batchData.vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "GlyphBatch VertexBuffer");
    batchData.instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(GlyphInstance), instance_format, "GlyphBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { batchData.vertex_buffer, batchData.instance_buffer };
    batchData.buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
        LLGL::BindingDescriptor("u_texture", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(3)),
        LLGL::BindingDescriptor("u_sampler", LLGL::ResourceType::Sampler, 0, LLGL::StageFlags::FragmentStage, LLGL::BindingSlot(backend.IsOpenGL() ? 3 : 4)),
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    std::vector<LLGL::VertexAttribute> vertex_attributes = vertex_format.attributes;
    vertex_attributes.insert(vertex_attributes.end(), instance_format.attributes.begin(), instance_format.attributes.end());

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
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "GlyphBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", vertex_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
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
        if (report->HasErrors()) SGE_LOG_ERROR("%s", report->GetText());
    }

    return batchData;
}

ShapeBatchData Renderer::InitShapeBatchPipeline() {
    const RenderBackend backend = m_backend;
    const auto& context = m_context;

    const Vertex vertices[] = {
        Vertex(0.0f, 0.0f),
        Vertex(0.0f, 1.0f),
        Vertex(1.0f, 0.0f),
        Vertex(1.0f, 1.0f),
    };

    LLGL::VertexFormat vertex_format;
    LLGL::VertexFormat instance_format;

    if (backend.IsGLSL()) {
        vertex_format.AppendAttribute({ "a_position", LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else if (backend.IsHLSL()) {
        vertex_format.AppendAttribute({ "Position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    } else {
        vertex_format.AppendAttribute({ "position",   LLGL::Format::RG32Float, 0, 0, sizeof(Vertex), 0, 0 });
    }

    if (backend.IsGLSL()) {
        instance_format.attributes = {
            {"i_position",           LLGL::Format::RGB32Float,  1,  offsetof(ShapeInstance,position),          sizeof(ShapeInstance), 1, 1 },
            {"i_size",               LLGL::Format::RG32Float,   2,  offsetof(ShapeInstance,size),              sizeof(ShapeInstance), 1, 1 },
            {"i_offset",             LLGL::Format::RG32Float,   3,  offsetof(ShapeInstance,offset),            sizeof(ShapeInstance), 1, 1 },
            {"i_color",              LLGL::Format::RGBA32Float, 4,  offsetof(ShapeInstance,color),             sizeof(ShapeInstance), 1, 1 },
            {"i_border_color",       LLGL::Format::RGBA32Float, 5,  offsetof(ShapeInstance,border_color),      sizeof(ShapeInstance), 1, 1 },
            {"i_border_radius",      LLGL::Format::RGBA32Float, 6,  offsetof(ShapeInstance,border_radius),     sizeof(ShapeInstance), 1, 1 },
            {"i_border_thickness",   LLGL::Format::R32Float,    7,  offsetof(ShapeInstance,border_thickness),  sizeof(ShapeInstance), 1, 1 },
            {"i_shape",              LLGL::Format::R8UInt,      8,  offsetof(ShapeInstance,shape),             sizeof(ShapeInstance), 1, 1 },
            {"i_flags",              LLGL::Format::R8UInt,      9,  offsetof(ShapeInstance,flags),             sizeof(ShapeInstance), 1, 1},
        };
    } else if (backend.IsHLSL()) {
        instance_format.attributes = {
            {"I_Position",         LLGL::Format::RGB32Float,  1,  offsetof(ShapeInstance,position),          sizeof(ShapeInstance), 1, 1 },
            {"I_Size",             LLGL::Format::RG32Float,   2,  offsetof(ShapeInstance,size),              sizeof(ShapeInstance), 1, 1 },
            {"I_Offset",           LLGL::Format::RG32Float,   3,  offsetof(ShapeInstance,offset),            sizeof(ShapeInstance), 1, 1 },
            {"I_Color",            LLGL::Format::RGBA32Float, 4,  offsetof(ShapeInstance,color),             sizeof(ShapeInstance), 1, 1 },
            {"I_BorderColor",      LLGL::Format::RGBA32Float, 5,  offsetof(ShapeInstance,border_color),      sizeof(ShapeInstance), 1, 1 },
            {"I_BorderRadius",     LLGL::Format::RGBA32Float, 6,  offsetof(ShapeInstance,border_radius),     sizeof(ShapeInstance), 1, 1 },
            {"I_BorderThickness",  LLGL::Format::R32Float,    7,  offsetof(ShapeInstance,border_thickness),  sizeof(ShapeInstance), 1, 1 },
            {"I_Shape",            LLGL::Format::R8UInt,      8,  offsetof(ShapeInstance,shape),             sizeof(ShapeInstance), 1, 1 },
            {"I_Flags",            LLGL::Format::R8UInt,      9,  offsetof(ShapeInstance,flags),             sizeof(ShapeInstance), 1, 1},
        };
    } else {
        instance_format.attributes = {
            {"i_position",           LLGL::Format::RGB32Float,  1,  offsetof(ShapeInstance,position),          sizeof(ShapeInstance), 1, 1 },
            {"i_size",               LLGL::Format::RG32Float,   2,  offsetof(ShapeInstance,size),              sizeof(ShapeInstance), 1, 1 },
            {"i_offset",             LLGL::Format::RG32Float,   3,  offsetof(ShapeInstance,offset),            sizeof(ShapeInstance), 1, 1 },
            {"i_color",              LLGL::Format::RGBA32Float, 4,  offsetof(ShapeInstance,color),             sizeof(ShapeInstance), 1, 1 },
            {"i_border_color",       LLGL::Format::RGBA32Float, 5,  offsetof(ShapeInstance,border_color),      sizeof(ShapeInstance), 1, 1 },
            {"i_border_radius",      LLGL::Format::RGBA32Float, 6,  offsetof(ShapeInstance,border_radius),     sizeof(ShapeInstance), 1, 1 },
            {"i_border_thickness",   LLGL::Format::R32Float,    7,  offsetof(ShapeInstance,border_thickness),  sizeof(ShapeInstance), 1, 1 },
            {"i_shape",              LLGL::Format::R8UInt,      8,  offsetof(ShapeInstance,shape),             sizeof(ShapeInstance), 1, 1 },
            {"i_flags",              LLGL::Format::R8UInt,      9,  offsetof(ShapeInstance,flags),             sizeof(ShapeInstance), 1, 1},
        };
    }

    ShapeBatchData batchData;

    batchData.buffer = checked_alloc<ShapeInstance>(MAX_QUADS);
    batchData.buffer_ptr = batchData.buffer;

    batchData.vertex_buffer = CreateVertexBufferInit(sizeof(vertices), vertices, vertex_format, "ShapeBatch VertexBuffer");
    batchData.instance_buffer = CreateVertexBuffer(MAX_QUADS * sizeof(ShapeInstance), instance_format, "ShapeBatch InstanceBuffer");

    LLGL::Buffer* buffers[] = { batchData.vertex_buffer, batchData.instance_buffer };
    batchData.buffer_array = context->CreateBufferArray(2, buffers);

    LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindings = {
        LLGL::BindingDescriptor(
            "GlobalUniformBuffer",
            LLGL::ResourceType::Buffer,
            LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,
            LLGL::BindingSlot(2)
        ),
    };

    LLGL::PipelineLayout* pipelineLayout = context->CreatePipelineLayout(pipelineLayoutDesc);

    std::vector<LLGL::VertexAttribute> vertex_attributes = vertex_format.attributes;
    vertex_attributes.insert(vertex_attributes.end(), instance_format.attributes.begin(), instance_format.attributes.end());

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
        // TODO
    } else if (backend.IsOpenGL()) {
        // TODO
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.debugName = "ShapeBatch Pipeline";
    pipelineDesc.vertexShader = CreateShader(*this, ShaderType::Vertex, vsSource, vsSize, "VS", vertex_attributes);
    pipelineDesc.fragmentShader = CreateShader(*this, ShaderType::Fragment, psSource, psSize, "PS");
    pipelineDesc.pipelineLayout = pipelineLayout;
    pipelineDesc.indexFormat = LLGL::Format::R16UInt;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
    pipelineDesc.renderPass = m_swap_chain->GetRenderPass();
    pipelineDesc.rasterizer.frontCCW = true;
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
        if (report->HasErrors()) SGE_LOG_ERROR("%s", report->GetText());
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

    if (m_context == nullptr) {
        SGE_LOG_ERROR("Couldn't load render system. %s", report.GetText());
        return false;
    }

    if (backend.IsOpenGL()) {
        delete (LLGL::OpenGLContextProfile*) rendererDesc.rendererConfig;
    }

    if (report.HasErrors()) {
        SGE_LOG_ERROR("%s", report.GetText());
        return false;
    }

    const auto& info = m_context->GetRendererInfo();

    SGE_LOG_INFO("Renderer:             %s", info.rendererName.c_str());
    SGE_LOG_INFO("Device:               %s", info.deviceName.c_str());
    SGE_LOG_INFO("Vendor:               %s", info.vendorName.c_str());
    SGE_LOG_INFO("Shading Language:     %s", info.shadingLanguageName.c_str());

    SGE_LOG_INFO("Extensions:");
    for (const auto& extension : info.extensionNames) {
        SGE_LOG_INFO("  %s", extension.c_str());
    }

    m_cache_pipelines = cache_pipelines;
    m_cache_pipeline_dir = cache_dir_path;

    if (cache_pipelines && (!fs::is_directory(cache_dir_path) || !fs::exists(cache_dir_path))) {
        fs::create_directories(cache_dir_path);
    }

    return true;
}

bool Renderer::Init(GLFWwindow* window, const LLGL::Extent2D& resolution, bool vsync, bool fullscreen) {
    ZoneScopedN("Renderer::Init");

    const LLGL::RenderSystemPtr& context = m_context;

    m_surface = std::make_shared<CustomSurface>(window, resolution);

    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.resolution = resolution;
    swapChainDesc.fullscreen = fullscreen;
    // swapChainDesc.samples = 4;

    m_swap_chain = context->CreateSwapChain(swapChainDesc, m_surface);
    m_swap_chain->SetVsyncInterval(vsync ? 1 : 0);

    LLGL::CommandBufferDescriptor command_buffer_desc;
    command_buffer_desc.numNativeBuffers = 3;

    m_command_buffer = context->CreateCommandBuffer(command_buffer_desc);
    m_command_queue = context->GetCommandQueue();

    LLGL::RenderPassDescriptor render_pass;
    render_pass.colorAttachments[0].format = m_swap_chain->GetColorFormat();
    render_pass.colorAttachments[0].loadOp = LLGL::AttachmentLoadOp::Undefined;
    render_pass.colorAttachments[0].storeOp = LLGL::AttachmentStoreOp::Store;
    render_pass.depthAttachment.format = m_swap_chain->GetDepthStencilFormat();
    render_pass.depthAttachment.loadOp = LLGL::AttachmentLoadOp::Load;
    render_pass.depthAttachment.storeOp = LLGL::AttachmentStoreOp::Store;
    render_pass.stencilAttachment.format = m_swap_chain->GetDepthStencilFormat();
    render_pass.stencilAttachment.loadOp = LLGL::AttachmentLoadOp::Load;
    render_pass.stencilAttachment.storeOp = LLGL::AttachmentStoreOp::Store;
    // render_pass.samples = 4;

    m_pass = m_context->CreateRenderPass(render_pass);

    m_constant_buffer = CreateConstantBuffer(sizeof(ProjectionsUniform), "ConstantBuffer");

    m_sprite_batch_data = InitSpriteBatchPipeline();
    m_ninepatch_batch_data = InitNinepatchBatchPipeline();
    m_glyph_batch_data = InitGlyphBatchPipeline();
    m_shape_batch_data = InitShapeBatchPipeline();

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

    m_sprite_instance_size = 0;
    m_glyph_instance_size = 0;
    m_ninepatch_instance_size = 0;
    m_shape_instance_size = 0;

    m_sprite_instance_count = 0;
    m_glyph_instance_count = 0;
    m_ninepatch_instance_count = 0;
    m_shape_instance_count = 0;
    m_batch_instance_count = 0;

    m_sprite_batch_data.buffer_ptr = m_sprite_batch_data.buffer;
    m_glyph_batch_data.buffer_ptr = m_glyph_batch_data.buffer;
    m_ninepatch_batch_data.buffer_ptr = m_ninepatch_batch_data.buffer;
    m_shape_batch_data.buffer_ptr = m_shape_batch_data.buffer;
}

void Renderer::BeginMainPass(const LLGL::ClearValue& clear_color, long flags) {
    m_current_framebuffer = m_swap_chain;
    m_current_pass = m_pass;

    m_command_buffer->BeginRenderPass(*m_swap_chain);
    // m_command_buffer->BeginRenderPass(*m_swap_chain, m_pass);
    m_command_buffer->Clear(flags, clear_color);
    m_command_buffer->SetViewport(m_swap_chain->GetResolution());
}

void Renderer::EndMainPass() {
    m_command_buffer->EndRenderPass();
}

void Renderer::End() {
    ZoneScopedN("Renderer::End");

    m_current_framebuffer = nullptr;
    m_current_pass = nullptr;

    m_command_buffer->End();
    m_command_queue->Submit(*m_command_buffer);

    m_swap_chain->Present();
}

void Renderer::ApplyBatchDrawCommands(sge::Batch& batch) {
    ZoneScopedN("Renderer::ApplyBatchDrawCommands");

    sge::Batch::FlushQueue& flush_queue = batch.flush_queue();

    auto* const commands = m_command_buffer;

    int prev_flush_data_type = -1;
    int prev_texture_id = -1;

    LLGL::PipelineState& sprite_pipeline = batch.depth_enabled()
        ? *m_sprite_batch_data.pipeline_depth
        : *m_sprite_batch_data.pipeline;

    size_t offset = 0;

    for (FlushData& flush_data : flush_queue) {
        if (prev_flush_data_type != static_cast<int>(flush_data.type)) {
            switch (flush_data.type) {
            case FlushDataType::Sprite:
                commands->SetVertexBufferArray(*m_sprite_batch_data.buffer_array);
                commands->SetPipelineState(sprite_pipeline);
                offset = batch.sprite_offset();
            break;

            case FlushDataType::Glyph:
                commands->SetVertexBufferArray(*m_glyph_batch_data.buffer_array);
                commands->SetPipelineState(*m_glyph_batch_data.pipeline);
                offset = batch.glyph_offset();
            break;

            case FlushDataType::NinePatch:
                commands->SetVertexBufferArray(*m_ninepatch_batch_data.buffer_array);
                commands->SetPipelineState(*m_ninepatch_batch_data.pipeline);
                offset = batch.ninepatch_offset();
            break;

            case FlushDataType::Shape:
                commands->SetVertexBufferArray(*m_shape_batch_data.buffer_array);
                commands->SetPipelineState(*m_shape_batch_data.pipeline);
                offset = batch.ninepatch_offset();
            break;
            }

            commands->SetResource(0, *m_constant_buffer);
        }

        if (flush_data.texture.has_value() && prev_texture_id != flush_data.texture->id()) {
            const Texture& texture = flush_data.texture.value();

            commands->SetResource(1, texture);
            commands->SetResource(2, texture.sampler());

            prev_texture_id = texture.id();
        }
        
        commands->DrawInstanced(4, 0, flush_data.count, offset + flush_data.offset);

        prev_flush_data_type = static_cast<int>(flush_data.type);
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
    uint32_t sprite_count = 0;
    uint32_t sprite_total_count = 0;
    uint32_t sprite_vertex_offset = 0;
    uint32_t sprite_remaining = batch.sprite_count();

    Texture glyph_prev_texture;
    uint32_t glyph_count = 0;
    uint32_t glyph_total_count = 0;
    uint32_t glyph_vertex_offset = 0;
    uint32_t glyph_remaining = batch.glyph_count();

    Texture ninepatch_prev_texture;
    uint32_t ninepatch_count = 0;
    uint32_t ninepatch_total_count = 0;
    uint32_t ninepatch_vertex_offset = 0;
    uint32_t ninepatch_remaining = batch.ninepatch_count();

    uint32_t shape_count = 0;
    uint32_t shape_total_count = 0;
    uint32_t shape_vertex_offset = 0;
    uint32_t shape_remaining = batch.shape_count();

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
            const DrawCommandSprite& sprite_data = draw_command.sprite_data();
            
            if (sprite_remaining == 0) continue;

            if (sprite_total_count == 0) {
                sprite_prev_texture = sprite_data.texture;
            }

            const uint32_t prev_texture_id = sprite_prev_texture.id();
            const uint32_t curr_texture_id = sprite_data.texture.id();

            const uint32_t current_order = sprite_data.order;

            if (sprite_count > 0 && prev_texture_id != curr_texture_id) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = current_order,
                    .type = FlushDataType::Sprite
                });
                sprite_count = 0;
                sprite_vertex_offset = sprite_total_count;
            }

            uint8_t flags = 0;
            flags |= sprite_data.ignore_camera_zoom << SpriteFlags::IgnoreCameraZoom;
            flags |= batch.is_ui() << SpriteFlags::UI;

            m_sprite_batch_data.buffer_ptr->position = sprite_data.position;
            m_sprite_batch_data.buffer_ptr->rotation = sprite_data.rotation;
            m_sprite_batch_data.buffer_ptr->size = sprite_data.size;
            m_sprite_batch_data.buffer_ptr->offset = sprite_data.offset;
            m_sprite_batch_data.buffer_ptr->uv_offset_scale = sprite_data.uv_offset_scale;
            m_sprite_batch_data.buffer_ptr->color = sprite_data.color;
            m_sprite_batch_data.buffer_ptr->outline_color = sprite_data.outline_color;
            m_sprite_batch_data.buffer_ptr->outline_thickness = sprite_data.outline_thickness;
            m_sprite_batch_data.buffer_ptr->flags = flags;
            m_sprite_batch_data.buffer_ptr++;

            ++sprite_count;
            ++sprite_total_count;
            --sprite_remaining;

            if (sprite_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_data.texture,
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = sprite_data.order,
                    .type = FlushDataType::Sprite
                });
                sprite_count = 0;
                sprite_vertex_offset = sprite_total_count;
            }

            sprite_prev_texture = sprite_data.texture;
        } break;
        case DrawCommand::DrawGlyph: {
            const DrawCommandGlyph& glyph_data = draw_command.glyph_data();

            if (glyph_remaining == 0) continue;

            if (glyph_total_count == 0) {
                glyph_prev_texture = glyph_data.texture;
            }

            const uint32_t current_order = glyph_data.order;

            if (glyph_count > 0 && glyph_prev_texture.id() != glyph_data.texture.id()) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = current_order,
                    .type = FlushDataType::Glyph
                });
                glyph_count = 0;
                glyph_vertex_offset = glyph_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.is_ui() << ShapeFlags::UI;

            m_glyph_batch_data.buffer_ptr->color = glyph_data.color;
            m_glyph_batch_data.buffer_ptr->pos = glyph_data.pos;
            m_glyph_batch_data.buffer_ptr->size = glyph_data.size;
            m_glyph_batch_data.buffer_ptr->tex_size = glyph_data.tex_size;
            m_glyph_batch_data.buffer_ptr->uv = glyph_data.tex_uv;
            m_glyph_batch_data.buffer_ptr->flags = flags;
            m_glyph_batch_data.buffer_ptr++;

            ++glyph_count;
            ++glyph_total_count;
            --glyph_remaining;

            if (glyph_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_data.texture,
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = glyph_data.order,
                    .type = FlushDataType::Glyph
                });
                glyph_count = 0;
                glyph_vertex_offset = glyph_total_count;
            }

            glyph_prev_texture = glyph_data.texture;
        } break;
        case DrawCommand::DrawNinePatch: {
            const DrawCommandNinePatch& ninepatch_data = draw_command.ninepatch_data();

            if (ninepatch_remaining == 0) continue;

            if (ninepatch_total_count == 0) {
                ninepatch_prev_texture = ninepatch_data.texture;
            }

            const uint32_t prev_texture_id = ninepatch_prev_texture.id();
            const uint32_t curr_texture_id = ninepatch_data.texture.id();

            const uint32_t current_order = ninepatch_data.order;

            if (ninepatch_count > 0 && prev_texture_id != curr_texture_id) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = current_order,
                    .type = FlushDataType::NinePatch
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            uint8_t flags = 0;
            flags |= batch.is_ui() << ShapeFlags::UI;

            m_ninepatch_batch_data.buffer_ptr->position = ninepatch_data.position;
            m_ninepatch_batch_data.buffer_ptr->rotation = ninepatch_data.rotation;
            m_ninepatch_batch_data.buffer_ptr->margin = ninepatch_data.margin;
            m_ninepatch_batch_data.buffer_ptr->size = ninepatch_data.size;
            m_ninepatch_batch_data.buffer_ptr->offset = ninepatch_data.offset;
            m_ninepatch_batch_data.buffer_ptr->source_size = ninepatch_data.source_size;
            m_ninepatch_batch_data.buffer_ptr->output_size = ninepatch_data.output_size;
            m_ninepatch_batch_data.buffer_ptr->uv_offset_scale = ninepatch_data.uv_offset_scale;
            m_ninepatch_batch_data.buffer_ptr->color = ninepatch_data.color;
            m_ninepatch_batch_data.buffer_ptr->flags = flags;
            m_ninepatch_batch_data.buffer_ptr++;

            ++ninepatch_count;
            ++ninepatch_total_count;
            --ninepatch_remaining;

            if (ninepatch_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_data.texture,
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = ninepatch_data.order,
                    .type = FlushDataType::NinePatch,
                });
                ninepatch_count = 0;
                ninepatch_vertex_offset = ninepatch_total_count;
            }

            ninepatch_prev_texture = ninepatch_data.texture;
        } break;
        case DrawCommand::DrawShape: {
            const DrawCommandShape& shape_data = draw_command.shape_data();

            if (shape_remaining == 0) continue;

            const uint32_t current_order = shape_data.order;

            uint8_t flags = 0;
            flags |= batch.is_ui() << ShapeFlags::UI;

            m_shape_batch_data.buffer_ptr->position = glm::vec3(shape_data.position, 0.0f);
            m_shape_batch_data.buffer_ptr->size = shape_data.size;
            m_shape_batch_data.buffer_ptr->offset = shape_data.offset;
            m_shape_batch_data.buffer_ptr->color = shape_data.color.to_vec4();
            m_shape_batch_data.buffer_ptr->border_color = shape_data.border_color.to_vec4();
            m_shape_batch_data.buffer_ptr->border_thickness = shape_data.border_thickness;
            m_shape_batch_data.buffer_ptr->border_radius = shape_data.border_radius;
            m_shape_batch_data.buffer_ptr->shape = shape_data.shape;
            m_shape_batch_data.buffer_ptr->flags = flags;
            m_shape_batch_data.buffer_ptr++;

            ++shape_count;
            ++shape_total_count;
            --shape_remaining;

            if (shape_remaining == 0 || current_order != next_order) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .offset = shape_vertex_offset,
                    .count = shape_count,
                    .order = shape_data.order,
                    .type = FlushDataType::Shape,
                });
                shape_count = 0;
                shape_vertex_offset = shape_total_count;
            }
        } break;
        }

        if (m_batch_instance_count + 1 >= MAX_QUADS) {
            if (sprite_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = sprite_prev_texture,
                    .offset = sprite_vertex_offset,
                    .count = sprite_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Sprite
                });
            }

            if (glyph_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = glyph_prev_texture,
                    .offset = glyph_vertex_offset,
                    .count = glyph_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Glyph
                });
            }

            if (ninepatch_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = ninepatch_prev_texture,
                    .offset = ninepatch_vertex_offset,
                    .count = ninepatch_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::NinePatch
                });
            }

            if (shape_count > 0) {
                flush_queue.push_back(FlushData {
                    .texture = std::nullopt,
                    .offset = shape_vertex_offset,
                    .count = shape_count,
                    .order = draw_command.order(),
                    .type = FlushDataType::Shape
                });
            }
        }

        ++m_batch_instance_count;
    }

    batch.set_draw_commands_done(i);

    const size_t sprite_size = sprite_total_count * sizeof(SpriteInstance);
    m_sprite_instance_size += sprite_size;
    m_sprite_instance_count += sprite_total_count;
    batch.set_sprite_count(sprite_remaining);

    const size_t glyph_size = glyph_total_count * sizeof(GlyphInstance);
    m_glyph_instance_size += glyph_size;
    m_glyph_instance_count += glyph_total_count;
    batch.set_glyph_count(glyph_remaining);

    const size_t ninepatch_size = ninepatch_total_count * sizeof(NinePatchInstance);
    m_ninepatch_instance_size += ninepatch_size;
    m_ninepatch_instance_count += ninepatch_total_count;
    batch.set_ninepatch_count(ninepatch_remaining);

    const size_t shape_size = shape_total_count * sizeof(ShapeInstance);
    m_shape_instance_size += shape_size;
    m_shape_instance_count += shape_total_count;
    batch.set_shape_count(shape_remaining);
}


void Renderer::PrepareBatch(sge::Batch& batch) {
    if (batch.draw_commands().empty()) return;

    batch.set_sprite_offset(m_sprite_instance_count);
    batch.set_glyph_offset(m_glyph_instance_count);
    batch.set_ninepatch_offset(m_ninepatch_instance_count);
    batch.set_shape_offset(m_shape_instance_count);

    SortBatchDrawCommands(batch);
    UpdateBatchBuffers(batch);
}

void Renderer::UploadBatchData() {
    if (m_sprite_instance_size > 0) {
        UpdateBuffer(m_sprite_batch_data.instance_buffer, m_sprite_batch_data.buffer, m_sprite_instance_size);
    }

    if (m_glyph_instance_size > 0) {
        UpdateBuffer(m_glyph_batch_data.instance_buffer, m_glyph_batch_data.buffer, m_glyph_instance_size);
    }

    if (m_ninepatch_instance_size > 0) {
        UpdateBuffer(m_ninepatch_batch_data.instance_buffer, m_ninepatch_batch_data.buffer, m_ninepatch_instance_size);
    }

    if (m_shape_instance_size > 0) {
        UpdateBuffer(m_shape_batch_data.instance_buffer, m_shape_batch_data.buffer, m_shape_instance_size);
    }
}

void Renderer::RenderBatch(sge::Batch& batch) {
    ZoneScopedN("Renderer::RenderBatch");

    const sge::Batch::DrawCommands& draw_commands = batch.draw_commands();

    if (draw_commands.empty()) return;

    ApplyBatchDrawCommands(batch);

    while (batch.draw_commands_done() < draw_commands.size()) {
        m_batch_instance_count = 0;

        m_sprite_instance_count = 0;
        m_sprite_instance_size = 0;
        m_sprite_batch_data.buffer_ptr = m_sprite_batch_data.buffer;
        batch.set_sprite_offset(m_sprite_instance_count);

        m_glyph_instance_count = 0;
        m_glyph_instance_size = 0;
        m_glyph_batch_data.buffer_ptr = m_glyph_batch_data.buffer;
        batch.set_glyph_offset(m_glyph_instance_count);

        m_ninepatch_instance_count = 0;
        m_ninepatch_instance_size = 0;
        m_ninepatch_batch_data.buffer_ptr = m_ninepatch_batch_data.buffer;
        batch.set_ninepatch_offset(m_ninepatch_instance_count);

        m_shape_instance_count = 0;
        m_shape_instance_size = 0;
        m_shape_batch_data.buffer_ptr = m_shape_batch_data.buffer;
        batch.set_shape_offset(m_shape_instance_count);

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
        SGE_LOG_ERROR("Failed to find shader '%s'", path.c_str());
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
                SGE_LOG_ERROR("Failed to create a shader. File: %s\nError: %s", path.c_str(), report->GetText());
                return nullptr;
            }
            
            SGE_LOG_INFO("%s", report->GetText());
        }
    }

    return shader;
}

#if SGE_DEBUG
void Renderer::PrintDebugInfo() {
    LLGL::FrameProfile profile;
    m_debugger->FlushProfile(&profile);

    SGE_LOG_DEBUG("Draw commands count: %u", profile.commandBufferRecord.drawCommands);
}
#endif

void Renderer::Terminate() {
    const auto& context = m_context;

    RESOURCE_RELEASE(m_sprite_batch_data.vertex_buffer)
    RESOURCE_RELEASE(m_sprite_batch_data.instance_buffer)
    RESOURCE_RELEASE(m_sprite_batch_data.buffer_array)
    RESOURCE_RELEASE(m_sprite_batch_data.pipeline)

    RESOURCE_RELEASE(m_glyph_batch_data.vertex_buffer)
    RESOURCE_RELEASE(m_glyph_batch_data.instance_buffer)
    RESOURCE_RELEASE(m_glyph_batch_data.buffer_array)
    RESOURCE_RELEASE(m_glyph_batch_data.pipeline)

    RESOURCE_RELEASE(m_ninepatch_batch_data.vertex_buffer)
    RESOURCE_RELEASE(m_ninepatch_batch_data.instance_buffer)
    RESOURCE_RELEASE(m_ninepatch_batch_data.buffer_array)
    RESOURCE_RELEASE(m_ninepatch_batch_data.pipeline)

    RESOURCE_RELEASE(m_constant_buffer);
    RESOURCE_RELEASE(m_command_buffer);
    RESOURCE_RELEASE(m_swap_chain);

    LLGL::RenderSystem::Unload(std::move(m_context));
}