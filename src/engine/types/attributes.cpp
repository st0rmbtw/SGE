#include <SGE/types/attributes.hpp>

std::vector<LLGL::VertexAttribute> sge::Attributes::ToLLGL(sge::RenderBackend backend, uint32_t start_location) const {
    if (m_items.empty()) return {};

    std::vector<LLGL::VertexAttribute> items;
    items.reserve(m_items.size());

    uint32_t stride = 0;
    uint32_t offset = 0;
    uint32_t alignment = LLGL::DataTypeSize(m_items[0].data_type);

    for (size_t i = 1; i < m_items.size(); ++i) {
        alignment = std::max(alignment, LLGL::DataTypeSize(m_items[i].data_type));
    }

    uint32_t location = start_location;

    uint32_t prev_alignment = LLGL::DataTypeSize(m_items[0].data_type);
    for (const Attribute& item : m_items) {
        const uint32_t item_alignment = LLGL::DataTypeSize(item.data_type);

        if (prev_alignment < item_alignment) 
            offset = (offset + (alignment - 1)) & -alignment;
        
        prev_alignment = item_alignment;

        LLGL::StringLiteral name = backend.IsHLSL() ? item.semantic_name : item.name;
        const uint32_t instance_divisor = item.type == Attribute::Type::PerInstance ? 1 : 0;

        items.emplace_back(std::move(name), item.format, location, offset, 0, item.slot, instance_divisor);

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