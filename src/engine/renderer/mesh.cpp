#include <SGE/renderer/context.hpp>
#include <SGE/renderer/mesh.hpp>

#include "utils.hpp"

sge::Mesh::Mesh(sge::RenderContext& context, const MeshDesc& desc) :
    m_topology{ desc.GetTopology() },
    m_front_face{ desc.GetFrontFace() }
 {
    LLGL::VertexFormat vertexFormat = ConvertMeshAttributesToLLGL(context.Backend(), desc.GetAttributes());

    uint64_t layoutHash = sge::HASH_INIT;
    HashVertexAttributes(layoutHash, vertexFormat.attributes);

    sge::Unique<LLGL::Buffer> vertexBuffer = context.CreateVertexBuffer(desc.GetVertexData(), desc.GetVertexDataSize(), vertexFormat.GetStride());
    sge::Unique<LLGL::Buffer> indexBuffer = nullptr;

    auto& indices = desc.GetIndices();
    if (!indices.IsNone()) {
        auto format = ConvertIndexFormatToLLGL(indices.GetFormat());
        indexBuffer = context.CreateIndexBuffer(indices.GetData(), indices.GetSize(), format);
    }

    uint32_t vertexCount = desc.GetVertexDataSize();
    if (vertexFormat.GetStride() > 0) {
        vertexCount /= vertexFormat.GetStride();
    }

    m_vertex_count = vertexCount;
    m_vertex_attributes = std::move(vertexFormat.attributes);
    m_vertex_buffer = std::move(vertexBuffer);
    m_index_buffer = std::move(indexBuffer);
    m_layout_hash = layoutHash;
    m_index_count = indices.GetCount();
    m_index_format = indices.GetFormat();
}