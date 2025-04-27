#include <SGE/types/attributes.hpp>

std::vector<LLGL::VertexAttribute> sge::Attributes::ToLLGL(sge::RenderBackend backend, uint32_t start_location) {
    if (m_items.empty()) return {};

    std::vector<LLGL::VertexAttribute> items;
    items.reserve(m_items.size());

    uint32_t stride = m_items[0].size;
    uint32_t offset = m_items[0].size;
    uint32_t alignment = LLGL::DataTypeSize(m_items[0].data_type);

    for (size_t i = 1; i < m_items.size(); ++i) {
        alignment = std::max(alignment, LLGL::DataTypeSize(m_items[i].data_type));
    }

    uint32_t prev_alignment = LLGL::DataTypeSize(m_items[0].data_type);
    for (size_t i = 1; i < m_items.size(); ++i) {
        Attribute& item = m_items[i];

        const uint32_t item_alignment = LLGL::DataTypeSize(item.data_type);

        if (prev_alignment < item_alignment) {
            offset = (offset + (alignment - 1)) & -alignment;
        }

        item.offset = offset;
        stride = std::max(stride, item.offset + item.size);
        offset += item.size;
        prev_alignment = item_alignment;
    }

    stride += (alignment - (stride % alignment)) % alignment;

    uint32_t location = start_location;
    
    for (const Attribute& item : m_items) {
        const LLGL::StringLiteral name = backend.IsHLSL() ? item.semantic_name : item.name;
        const uint32_t instance_divisor = item.type == Attribute::Type::PerInstance ? 1 : 0;

        items.emplace_back(name, item.format, location, item.offset, stride, item.slot, instance_divisor);

        ++location;
    }

    return items;
}