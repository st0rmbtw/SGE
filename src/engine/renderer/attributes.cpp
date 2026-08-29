#include <LLGL/VertexAttribute.h>
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

    std::vector<LLGL::VertexAttribute> outAttributes;
    outAttributes.reserve(attributes.size());

    uint32_t stride = 0;
    uint32_t offset = 0;
    uint32_t alignment = 0;

    for (const auto& attr : attributes) {
        alignment = std::max(alignment, LLGL::DataTypeSize(attr.dataType));
    }

    uint32_t prevAlignment = LLGL::DataTypeSize(attributes[0].dataType);
    for (const Attribute& item : attributes) {
        const uint32_t itemAlignment = LLGL::DataTypeSize(item.dataType);
        if (prevAlignment < itemAlignment)
            offset = (offset + (alignment - 1)) & -alignment;

        prevAlignment = itemAlignment;

        const uint32_t instanceDivisor = item.type == Attribute::Type::PerInstance ? 1 : 0;
        auto name = backend.IsHLSL() ? item.semanticName : item.name;

        outAttributes.emplace_back(std::move(name), VertexFormatToLLGLFormat(item.format), location, offset, 0, item.slot, instanceDivisor);

        stride = std::max(stride, offset + item.size);
        offset += item.size;

        ++location;
    }

    stride += (alignment - (stride % alignment)) % alignment;

    for (LLGL::VertexAttribute& item : outAttributes) {
        item.stride = stride;
    }

    return LLGL::VertexFormat(outAttributes);
}