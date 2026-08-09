#ifndef RENDERER_UTILS_HPP_
#define RENDERER_UTILS_HPP_

#include <LLGL/Format.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Utils/VertexFormat.h>

#include <SGE/assert.hpp>
#include <SGE/renderer/material.hpp>
#include <SGE/renderer/mesh.hpp>
#include <SGE/renderer/vertex_attribute.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/utils/hash.hpp>


namespace sge {

constexpr inline LLGL::Format VertexFormatToLLGLFormat(sge::VertexFormat format) {
    switch (format) {
    case sge::VertexFormat::Uint8: return LLGL::Format::R8UInt;
    case sge::VertexFormat::Uint8x2: return LLGL::Format::RG8UInt;
    case sge::VertexFormat::Uint8x3: return LLGL::Format::RGB8UInt;
    case sge::VertexFormat::Uint8x4: return LLGL::Format::RGBA8UInt;
    case sge::VertexFormat::Sint8: return LLGL::Format::R8SInt;
    case sge::VertexFormat::Sint8x2: return LLGL::Format::RG8SInt;
    case sge::VertexFormat::Sint8x3: return LLGL::Format::RGB8SInt;
    case sge::VertexFormat::Sint8x4: return LLGL::Format::RGBA8SInt;
    case sge::VertexFormat::Unorm8:  return LLGL::Format::R8UNorm;
    case sge::VertexFormat::Unorm8x2: return LLGL::Format::RG8UNorm;
    case sge::VertexFormat::Unorm8x3: return LLGL::Format::RGB8UNorm;
    case sge::VertexFormat::Unorm8x4: return LLGL::Format::RGBA8UNorm;
    case sge::VertexFormat::Snorm8:  return LLGL::Format::R8SNorm;
    case sge::VertexFormat::Snorm8x2: return LLGL::Format::RG8SNorm;
    case sge::VertexFormat::Snorm8x3: return LLGL::Format::RGB8SNorm;
    case sge::VertexFormat::Snorm8x4: return LLGL::Format::RGBA8SNorm;
    case sge::VertexFormat::Uint16:  return LLGL::Format::R16UInt;
    case sge::VertexFormat::Uint16x2: return LLGL::Format::RG16UInt;
    case sge::VertexFormat::Uint16x3: return LLGL::Format::RGB16UInt;
    case sge::VertexFormat::Uint16x4: return LLGL::Format::RGBA16UInt;
    case sge::VertexFormat::Sint16:  return LLGL::Format::R16SInt;
    case sge::VertexFormat::Sint16x2: return LLGL::Format::RG16SInt;
    case sge::VertexFormat::Sint16x3: return LLGL::Format::RGB16SInt;
    case sge::VertexFormat::Sint16x4: return LLGL::Format::RGBA16SInt;
    case sge::VertexFormat::Unorm16:  return LLGL::Format::R16UNorm;
    case sge::VertexFormat::Unorm16x2: return LLGL::Format::RG16UNorm;
    case sge::VertexFormat::Unorm16x3: return LLGL::Format::RGB16UNorm;
    case sge::VertexFormat::Unorm16x4: return LLGL::Format::RGBA16UNorm;
    case sge::VertexFormat::Snorm16:  return LLGL::Format::R16SNorm;
    case sge::VertexFormat::Snorm16x2: return LLGL::Format::RG16SNorm;
    case sge::VertexFormat::Snorm16x3: return LLGL::Format::RGB16SNorm;
    case sge::VertexFormat::Snorm16x4: return LLGL::Format::RGBA16SNorm;
    case sge::VertexFormat::Float16:  return LLGL::Format::R16Float;
    case sge::VertexFormat::Float16x2: return LLGL::Format::RG16Float;
    case sge::VertexFormat::Float16x3: return LLGL::Format::RGB16Float;
    case sge::VertexFormat::Float16x4: return LLGL::Format::RGBA16Float;
    case sge::VertexFormat::Float32:  return LLGL::Format::R32Float;
    case sge::VertexFormat::Float32x2: return LLGL::Format::RG32Float;
    case sge::VertexFormat::Float32x3: return LLGL::Format::RGB32Float;
    case sge::VertexFormat::Float32x4: return LLGL::Format::RGBA32Float;
    case sge::VertexFormat::Uint32: return LLGL::Format::R32UInt;
    case sge::VertexFormat::Uint32x2: return LLGL::Format::RG32UInt;
    case sge::VertexFormat::Uint32x3: return LLGL::Format::RGB32UInt;
    case sge::VertexFormat::Uint32x4: return LLGL::Format::RGBA32UInt;
    case sge::VertexFormat::Sint32: return LLGL::Format::R32SInt;
    case sge::VertexFormat::Sint32x2: return LLGL::Format::RG32SInt;
    case sge::VertexFormat::Sint32x3: return LLGL::Format::RGB32SInt;
    case sge::VertexFormat::Sint32x4: return LLGL::Format::RGBA32SInt;
    case sge::VertexFormat::Float64: return LLGL::Format::R64Float;
    case sge::VertexFormat::Float64x2: return LLGL::Format::RG64Float;
    case sge::VertexFormat::Float64x3: return LLGL::Format::RGB64Float;
    case sge::VertexFormat::Float64x4: return LLGL::Format::RGBA64Float;
    case sge::VertexFormat::Unorm10_10_10_2: return LLGL::Format::RGB10A2UNorm;
    case sge::VertexFormat::Unorm8x4Bgra: return LLGL::Format::BGRA8UNorm;
    default: SGE_UNREACHABLE();
    }
}

constexpr inline LLGL::PrimitiveTopology ConvertTopologyToLLGL(sge::PrimitiveTopology topology) {
    switch (topology) {
    case sge::PrimitiveTopology::PointList: return LLGL::PrimitiveTopology::PointList;
    case sge::PrimitiveTopology::TriangleList: return LLGL::PrimitiveTopology::TriangleList;
    case sge::PrimitiveTopology::TriangleStrip: return LLGL::PrimitiveTopology::TriangleStrip;
    case sge::PrimitiveTopology::TriangleListAdjacency: return LLGL::PrimitiveTopology::TriangleListAdjacency;
    case sge::PrimitiveTopology::LineList: return LLGL::PrimitiveTopology::LineList;
    case sge::PrimitiveTopology::LineStrip: return LLGL::PrimitiveTopology::LineStrip;
    case sge::PrimitiveTopology::LineListAdjacency: return LLGL::PrimitiveTopology::LineListAdjacency;
    case sge::PrimitiveTopology::TriangleStripAdjacency: return LLGL::PrimitiveTopology::TriangleStripAdjacency;
    }
}

constexpr inline LLGL::Format ConvertIndexFormatToLLGL(sge::IndexFormat format) {
    switch (format) {
    case sge::IndexFormat::None: return LLGL::Format::Undefined;
    case sge::IndexFormat::U16: return LLGL::Format::R16UInt;
    case sge::IndexFormat::U32: return LLGL::Format::R32UInt;
    }
}

constexpr inline LLGL::CullMode ConvertCullModeFormatToLLGL(sge::CullMode cullMode) {
    switch (cullMode) {
    case CullMode::None: return LLGL::CullMode::Disabled;
    case CullMode::Back: return LLGL::CullMode::Back;
    case CullMode::Front: return LLGL::CullMode::Front;
    }
}

constexpr inline LLGL::BlendTargetDescriptor ConvertBlendModeToLLGL(sge::BlendMode blendMode) {
    switch (blendMode) {
    case sge::BlendMode::Opaque:
        return LLGL::BlendTargetDescriptor {
            .blendEnabled = false
        };
    case sge::BlendMode::AlphaBlend:
        return LLGL::BlendTargetDescriptor {
            .blendEnabled = true,
            .srcColor = LLGL::BlendOp::SrcAlpha,
            .dstColor = LLGL::BlendOp::InvSrcAlpha,
            .srcAlpha = LLGL::BlendOp::SrcAlpha,
            .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
        };
    case sge::BlendMode::Additive:
        return LLGL::BlendTargetDescriptor {
            .blendEnabled = true,
            .srcColor = LLGL::BlendOp::SrcAlpha,
            .dstColor = LLGL::BlendOp::One,
            .srcAlpha = LLGL::BlendOp::SrcAlpha,
            .dstAlpha = LLGL::BlendOp::One,
        };
    case sge::BlendMode::PremultipliedAlpha:
        return LLGL::BlendTargetDescriptor {
            .blendEnabled = true,
            .srcColor = LLGL::BlendOp::One,
            .dstColor = LLGL::BlendOp::InvSrcAlpha,
            .srcAlpha = LLGL::BlendOp::One,
            .dstAlpha = LLGL::BlendOp::InvSrcAlpha,
        };
    }
}

inline LLGL::VertexFormat ConvertVertexAttributesToLLGL(sge::RenderBackend backend, const std::vector<sge::VertexAttribute>& attributes) {
    LLGL::VertexFormat vertexFormat;
    uint32_t location = 0;
    for (const auto& attribute : attributes) {
        auto& name = backend.IsHLSL() ? attribute.semanticName : attribute.name;
        auto format = VertexFormatToLLGLFormat(attribute.format);
        vertexFormat.AppendAttribute(LLGL::VertexAttribute(name, format, location++, 0));
    }
    return vertexFormat;
}

inline void HashVertexFormat(uint64_t& hash, const LLGL::VertexFormat& vertexFormat) {
    for (const auto& attribute : vertexFormat.attributes) {
        sge::hash_combine(hash, attribute.format);
        sge::hash_combine(hash, attribute.location);
    }
}

} // namespace sge

#endif // RENDERER_UTILS_HPP_