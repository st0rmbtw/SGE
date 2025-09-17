#ifndef _SGE_TYPES_ATTRIBUTES_HPP_
#define _SGE_TYPES_ATTRIBUTES_HPP_

#include <initializer_list>
#include <vector>

#include <LLGL/Format.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/Utils/VertexFormat.h>

#include <SGE/types/backend.hpp>

namespace sge {

class Attributes;

class Attribute {
private:
    friend class Attributes;

    enum class Type : uint8_t {
        PerVertex = 0,
        PerInstance
    };

public:
    static Attribute Vertex(LLGL::Format format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
        return Attribute(format, Type::PerVertex, name, semantic_name, slot);
    }

    static Attribute Instance(LLGL::Format format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
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
    Attributes(sge::RenderBackend backend, uint32_t start_location, std::vector<Attribute> items) :
        m_items(std::move(items)),
        m_start_location(start_location)
    {
        for (Attribute& item : m_items) {
            if (backend.IsHLSL()) {
                item.name = item.semantic_name;
            }
        }
    }

    Attributes(sge::RenderBackend backend, uint32_t start_location, std::initializer_list<Attribute> items) : Attributes(backend, start_location, std::vector(items)) {}

    Attributes(sge::RenderBackend backend, std::initializer_list<Attribute> items) : Attributes(backend, 0, items) {}
    Attributes(sge::RenderBackend backend, std::vector<Attribute> items) : Attributes(backend, 0, items) {}

    [[nodiscard]]
    inline constexpr size_t size() const noexcept {
        return m_items.size();
    }

    [[nodiscard]]
    std::vector<LLGL::VertexAttribute> ToLLGL() const;

    operator LLGL::VertexFormat() const {
        LLGL::VertexFormat vertex_format;
        vertex_format.attributes = ToLLGL();
        return vertex_format;
    }

private:
    std::vector<Attribute> m_items;
    uint32_t m_start_location = 0;
};

}

#endif