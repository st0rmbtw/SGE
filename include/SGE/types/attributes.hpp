#ifndef _SGE_TYPES_ATTRIBUTES_HPP_
#define _SGE_TYPES_ATTRIBUTES_HPP_

#include "SGE/assert.hpp"
#include <initializer_list>
#include <span>
#include <utility>
#include <vector>

#include <LLGL/Format.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/VertexAttribute.h>

#include <SGE/types/backend.hpp>

namespace sge {

enum class VertexFormat : uint8_t {
    Uint8 = 0,
    Uint8x2,
    Uint8x3,
    Uint8x4,
    Sint8,
    Sint8x2,
    Sint8x3,
    Sint8x4,
    Unorm8,
    Unorm8x2,
    Unorm8x3,
    Unorm8x4,
    Snorm8,
    Snorm8x2,
    Snorm8x3,
    Snorm8x4,
    Uint16,
    Uint16x2,
    Uint16x3,
    Uint16x4,
    Sint16,
    Sint16x2,
    Sint16x3,
    Sint16x4,
    Unorm16,
    Unorm16x2,
    Unorm16x3,
    Unorm16x4,
    Snorm16,
    Snorm16x2,
    Snorm16x3,
    Snorm16x4,
    Float16,
    Float16x2,
    Float16x3,
    Float16x4,
    Float32,
    Float32x2,
    Float32x3,
    Float32x4,
    Uint32,
    Uint32x2,
    Uint32x3,
    Uint32x4,
    Sint32,
    Sint32x2,
    Sint32x3,
    Sint32x4,
    Float64,
    Float64x2,
    Float64x3,
    Float64x4,
    Unorm10_10_10_2,
    Unorm8x4Bgra,
};

inline constexpr LLGL::Format VertexFormatToLLGLFormat(sge::VertexFormat format) {
    switch (format) {
    case VertexFormat::Uint8: return LLGL::Format::R8UInt;
    case VertexFormat::Uint8x2: return LLGL::Format::RG8UInt;
    case VertexFormat::Uint8x3: return LLGL::Format::RGB8UInt;
    case VertexFormat::Uint8x4: return LLGL::Format::RGBA8UInt;
    case VertexFormat::Sint8: return LLGL::Format::R8SInt;
    case VertexFormat::Sint8x2: return LLGL::Format::RG8SInt;
    case VertexFormat::Sint8x3: return LLGL::Format::RGB8SInt;
    case VertexFormat::Sint8x4: return LLGL::Format::RGBA8SInt;
    case VertexFormat::Unorm8:  return LLGL::Format::R8UNorm;
    case VertexFormat::Unorm8x2: return LLGL::Format::RG8UNorm;
    case VertexFormat::Unorm8x3: return LLGL::Format::RGB8UNorm;
    case VertexFormat::Unorm8x4: return LLGL::Format::RGBA8UNorm;
    case VertexFormat::Snorm8:  return LLGL::Format::R8SNorm;
    case VertexFormat::Snorm8x2: return LLGL::Format::RG8SNorm;
    case VertexFormat::Snorm8x3: return LLGL::Format::RGB8SNorm;
    case VertexFormat::Snorm8x4: return LLGL::Format::RGBA8SNorm;
    case VertexFormat::Uint16:  return LLGL::Format::R16UInt;
    case VertexFormat::Uint16x2: return LLGL::Format::RG16UInt;
    case VertexFormat::Uint16x3: return LLGL::Format::RGB16UInt;
    case VertexFormat::Uint16x4: return LLGL::Format::RGBA16UInt;
    case VertexFormat::Sint16:  return LLGL::Format::R16SInt;
    case VertexFormat::Sint16x2: return LLGL::Format::RG16SInt;
    case VertexFormat::Sint16x3: return LLGL::Format::RGB16SInt;
    case VertexFormat::Sint16x4: return LLGL::Format::RGBA16SInt;
    case VertexFormat::Unorm16:  return LLGL::Format::R16UNorm;
    case VertexFormat::Unorm16x2: return LLGL::Format::RG16UNorm;
    case VertexFormat::Unorm16x3: return LLGL::Format::RGB16UNorm;
    case VertexFormat::Unorm16x4: return LLGL::Format::RGBA16UNorm;
    case VertexFormat::Snorm16:  return LLGL::Format::R16SNorm;
    case VertexFormat::Snorm16x2: return LLGL::Format::RG16SNorm;
    case VertexFormat::Snorm16x3: return LLGL::Format::RGB16SNorm;
    case VertexFormat::Snorm16x4: return LLGL::Format::RGBA16SNorm;
    case VertexFormat::Float16:  return LLGL::Format::R16Float;
    case VertexFormat::Float16x2: return LLGL::Format::RG16Float;
    case VertexFormat::Float16x3: return LLGL::Format::RGB16Float;
    case VertexFormat::Float16x4: return LLGL::Format::RGBA16Float;
    case VertexFormat::Float32:  return LLGL::Format::R32Float;
    case VertexFormat::Float32x2: return LLGL::Format::RG32Float;
    case VertexFormat::Float32x3: return LLGL::Format::RGB32Float;
    case VertexFormat::Float32x4: return LLGL::Format::RGBA32Float;
    case VertexFormat::Uint32: return LLGL::Format::R32UInt;
    case VertexFormat::Uint32x2: return LLGL::Format::RG32UInt;
    case VertexFormat::Uint32x3: return LLGL::Format::RGB32UInt;
    case VertexFormat::Uint32x4: return LLGL::Format::RGBA32UInt;
    case VertexFormat::Sint32: return LLGL::Format::R32SInt;
    case VertexFormat::Sint32x2: return LLGL::Format::RG32SInt;
    case VertexFormat::Sint32x3: return LLGL::Format::RGB32SInt;
    case VertexFormat::Sint32x4: return LLGL::Format::RGBA32SInt;
    case VertexFormat::Float64: return LLGL::Format::R64Float;
    case VertexFormat::Float64x2: return LLGL::Format::RG64Float;
    case VertexFormat::Float64x3: return LLGL::Format::RGB64Float;
    case VertexFormat::Float64x4: return LLGL::Format::RGBA64Float;
    case VertexFormat::Unorm10_10_10_2: return LLGL::Format::RGB10A2UNorm;
    case VertexFormat::Unorm8x4Bgra: return LLGL::Format::BGRA8UNorm;
    default: SGE_UNREACHABLE();
    }
}

struct Attribute {
    enum class Type : uint8_t {
        PerVertex = 0,
        PerInstance
    };

    Attribute(Type type, sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot) :
        name(std::move(name)),
        semanticName(std::move(semantic_name)),
        format(format),
        slot(slot),
        type(type)
    {
        const LLGL::FormatAttributes attrs = LLGL::GetFormatAttribs(VertexFormatToLLGLFormat(format));
        dataType = attrs.dataType;
        size = attrs.bitSize / 8;
    }

    static Attribute Vertex(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
        return Attribute(Type::PerVertex, format, std::move(name), std::move(semantic_name), slot);
    }

    static Attribute Instance(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
        return Attribute(Type::PerInstance, format, std::move(name), std::move(semantic_name), slot);
    }

    LLGL::StringLiteral name;
    LLGL::StringLiteral semanticName;

    sge::VertexFormat format;
    LLGL::DataType dataType = LLGL::DataType::Undefined;
    uint32_t size = 0;
    uint32_t slot = 0;
    Type type = Type::PerVertex;
};

std::vector<LLGL::VertexAttribute> VertexAttributes(sge::RenderBackend backend, uint8_t startLocation, std::span<const Attribute> attributes);

inline std::vector<LLGL::VertexAttribute> VertexAttributes(sge::RenderBackend backend, std::span<const Attribute> attributes) {
    return VertexAttributes(backend, 0, attributes);
}

inline std::vector<LLGL::VertexAttribute> VertexAttributes(sge::RenderBackend backend, uint8_t startLocation, std::initializer_list<Attribute> attributes) {
    return VertexAttributes(backend, startLocation, { attributes.begin(), attributes.end() });
}

inline std::vector<LLGL::VertexAttribute> VertexAttributes(sge::RenderBackend backend, std::initializer_list<Attribute> attributes) {
    return VertexAttributes(backend, 0, { attributes.begin(), attributes.end() });
}

} // namespace sge

#endif