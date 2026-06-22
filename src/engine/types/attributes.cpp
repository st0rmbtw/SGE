#include <SGE/types/attributes.hpp>

#include <span>

std::vector<LLGL::VertexAttribute> sge::VertexAttributes(sge::RenderBackend backend, uint8_t location, std::span<const Attribute> attributes) {
    if (attributes.empty()) return {};

    std::vector<LLGL::VertexAttribute> items;
    items.reserve(attributes.size());

    uint32_t stride = 0;
    uint32_t offset = 0;
    uint32_t alignment = LLGL::DataTypeSize(attributes[0].dataType);

    for (size_t i = 1; i < attributes.size(); ++i) {
        alignment = std::max(alignment, LLGL::DataTypeSize(attributes[i].dataType));
    }

    uint32_t prev_alignment = LLGL::DataTypeSize(attributes[0].dataType);
    for (const Attribute& item : attributes) {
        const uint32_t item_alignment = LLGL::DataTypeSize(item.dataType);

        if (prev_alignment < item_alignment)
            offset = (offset + (alignment - 1)) & -alignment;

        prev_alignment = item_alignment;

        const uint32_t instance_divisor = item.type == Attribute::Type::PerInstance ? 1 : 0;

        LLGL::StringLiteral name = backend.IsHLSL() ? item.semanticName : item.name;

        items.emplace_back(name, VertexFormatToLLGLFormat(item.format), location, offset, 0, item.slot, instance_divisor);

        stride = std::max(stride, offset + item.size);
        offset += item.size;

        ++location;
    }

    stride += (alignment - (stride % alignment)) % alignment;

    for (LLGL::VertexAttribute& item : items) {
        item.stride = stride;
    }

    return items;
}