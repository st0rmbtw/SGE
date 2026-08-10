#include <SGE/renderer/attributes.hpp>

#include "utils.hpp"

sge::Attribute::Attribute(Type type, sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot) :
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

LLGL::VertexFormat sge::VertexAttributes(sge::RenderBackend backend, uint8_t location, std::span<const Attribute> attributes) {
    if (attributes.empty()) return {};

    LLGL::VertexFormat vertexFormat;
    vertexFormat.attributes.reserve(attributes.size());
    
    for (const auto& item : attributes) {
        LLGL::StringLiteral name = backend.IsHLSL() ? item.semanticName : item.name;
        LLGL::Format format = VertexFormatToLLGLFormat(item.format);
        uint32_t instanceDivisor = item.type == Attribute::Type::PerInstance ? 1 : 0;
        vertexFormat.AppendAttribute(LLGL::VertexAttribute(name, format, 0, instanceDivisor));

        auto& attribute = vertexFormat.attributes.back();
        attribute.slot = item.slot;
        attribute.location = location++;
    }

    return vertexFormat;
}