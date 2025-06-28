#ifndef _SGE_TYPES_ATTRIBUTES_HPP_
#define _SGE_TYPES_ATTRIBUTES_HPP_

#include "LLGL/Format.h"
#include <initializer_list>
#include <vector>
#include <LLGL/VertexAttribute.h>
#include <SGE/types/backend.hpp>
#include <SGE/defines.hpp>

_SGE_BEGIN

class Attributes;

class Attribute {
private:
    friend class Attributes;

    enum class Type : uint8_t {
        PerVertex = 0,
        PerInstance
    };

public:
    static Attribute Vertex(LLGL::Format format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) {
        return Attribute(format, Type::PerVertex, name, semantic_name, slot);
    }

    static Attribute Instance(LLGL::Format format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) {
        return Attribute(format, Type::PerInstance, name, semantic_name, slot);
    }

private:
    Attribute(LLGL::Format format, Type type, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot) :
        name(std::move(name)),
        semantic_name(std::move(semantic_name)),
        format(format),
        slot(slot),
        type(type)
    {
        const LLGL::FormatAttributes attrs = LLGL::GetFormatAttribs(format);
        data_type = attrs.dataType;
        size = attrs.bitSize / 8;
    }

    LLGL::StringLiteral name;
    LLGL::StringLiteral semantic_name;

    LLGL::Format format = LLGL::Format::Undefined;
    LLGL::DataType data_type = LLGL::DataType::Undefined;
    uint32_t size = 0;
    uint32_t slot = 0;
    Type type = Type::PerVertex;
};

class Attributes {
public:
    Attributes(std::initializer_list<Attribute> items) : m_items(items) {}
    Attributes(std::vector<Attribute> items) : m_items(std::move(items)) {}

    [[nodiscard]]
    std::vector<LLGL::VertexAttribute> ToLLGL(sge::RenderBackend backend, uint32_t start_location = 0) const;

private:
    std::vector<Attribute> m_items;
};

_SGE_END

#endif