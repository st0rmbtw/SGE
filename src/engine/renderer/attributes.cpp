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

    for (const Attribute& item : attributes) {
        const uint32_t instanceDivisor = item.type == Attribute::Type::PerInstance ? 1 : 0;
        auto name = backend.IsHLSL() ? item.semanticName : item.name;

        outAttributes.emplace_back(std::move(name), VertexFormatToLLGLFormat(item.format), location, 0, 0, item.slot, instanceDivisor);

        ++location;
    }

    uint32_t alignment = 0;
    uint32_t offset = 0;
    uint32_t stride = 0;


    size_t startIdx = 0;

    while (startIdx < outAttributes.size()) {
        size_t endIdx = startIdx;
        for (; endIdx < outAttributes.size(); ++endIdx) {
            if (outAttributes[startIdx].slot != outAttributes[endIdx].slot)
                break;

            const LLGL::FormatAttributes& attrs = LLGL::GetFormatAttribs(outAttributes[endIdx].format);

            outAttributes[endIdx].offset = offset;

            const uint16_t size = attrs.bitSize / 8;
            stride = std::max(stride, offset + size);
            alignment = std::max(alignment, LLGL::DataTypeSize(attrs.dataType));
            offset += size;
        }

        stride += (alignment - (stride % alignment)) % alignment;

        for (size_t idx = startIdx; idx < endIdx; ++idx) {
            outAttributes[idx].stride = stride;
        }

        alignment = 0;
        offset = 0;
        stride = 0;
        startIdx = endIdx;
    }

    return LLGL::VertexFormat(outAttributes);
}