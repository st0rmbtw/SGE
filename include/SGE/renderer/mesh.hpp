#ifndef SGE_RENDERER_MESH_HPP_
#define SGE_RENDERER_MESH_HPP_

#include <span>
#include <vector>

#include <SGE/assert.hpp>
#include <SGE/renderer/vertex_format.hpp>
#include <SGE/utils/alloc.hpp>
#include <SGE/utils/containers/heaparray.hpp>

#include <LLGL/Container/StringLiteral.h>
#include <LLGL/PipelineStateFlags.h>


namespace sge {

enum class IndexFormat : uint8_t {
    None = 0,
    U16,
    U32,
};

enum class PrimitiveTopology : uint8_t {
    PointList,
    TriangleList,
    TriangleStrip,
    TriangleListAdjacency,
    LineList,
    LineStrip,
    LineListAdjacency,
    TriangleStripAdjacency
};

enum class FrontFace : uint8_t {
    CCW,
    CW,
};

class Indices {
public:
    Indices() = default;

    explicit Indices(IndexFormat format, uint32_t count, std::unique_ptr<uint16_t[]> data, size_t size) :
        m_data(std::move(data)),
        m_size(size),
        m_count(count),
        m_format(format) {}

    static inline Indices U32(std::span<uint32_t> indices) {
        auto data = std::make_unique<uint16_t[]>(indices.size() * 2);
        memcpy(data.get(), indices.data(), indices.size_bytes());
        return Indices(IndexFormat::U32, indices.size(), std::move(data), indices.size_bytes());
    }

    static inline Indices U16(std::span<uint16_t> indices) {
        auto data = std::make_unique<uint16_t[]>(indices.size());
        memcpy(data.get(), indices.data(), indices.size_bytes());
        return Indices(IndexFormat::U16, indices.size(), std::move(data), indices.size_bytes());
    }

    [[nodiscard]]
    bool IsNone() const {
        return m_format == IndexFormat::None;
    }

    [[nodiscard]]
    const void* GetData() const {
        return m_data.get();
    }

    [[nodiscard]]
    size_t GetSize() const {
        return m_size;
    }

    [[nodiscard]]
    uint32_t GetCount() const {
        return m_count;
    }

    [[nodiscard]]
    IndexFormat GetFormat() const {
        return m_format;
    }

private:
    std::unique_ptr<uint16_t[]> m_data = nullptr;
    size_t m_size = 0;
    uint32_t m_count = 0;
    IndexFormat m_format = IndexFormat::None;
};

struct MeshAttribute {
    explicit MeshAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot) :
        name(std::move(name)),
        semanticName(std::move(semantic_name)),
        slot(slot),
        format(format)
    {
    }

    LLGL::StringLiteral name;
    LLGL::StringLiteral semanticName;

    uint32_t slot = 0;
    sge::VertexFormat format;
};

class Mesh {
public:
    Mesh() = default;

    Mesh&& AddAttribute(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) && {
        m_attributes.emplace_back(format, std::move(name), std::move(semantic_name), slot);
        return std::move(*this);
    }

    Mesh&& AddAttribute(sge::MeshAttribute attribute) && {
        m_attributes.push_back(std::move(attribute));
        return std::move(*this);
    }

    Mesh&& SetTopology(sge::PrimitiveTopology topology) && {
        m_topology = topology;
        return std::move(*this);
    }

    Mesh&& SetFrontFace(sge::FrontFace front_face) && {
        m_front_face = front_face;
        return std::move(*this);
    }

    Mesh&& SetIndices(sge::Indices indices) && {
        m_indices = std::move(indices);
        return std::move(*this);
    }

    template <typename TVertex>
    Mesh&& SetVertices(std::initializer_list<TVertex> vertices) && {
        return std::move(*this).SetVertices(std::span<const TVertex>(vertices.begin(), vertices.end()));
    }

    template <typename TContainer>
    Mesh&& SetVertices(const TContainer& vertices) && {
        using ElementType = std::ranges::range_value_t<TContainer>;
        const void* data = std::data(vertices);
        const size_t size = std::size(vertices);
        const size_t size_bytes = std::size(vertices) * sizeof(ElementType);
        return std::move(*this).SetVertexData(data, size_bytes, static_cast<uint32_t>(size));
    }

    Mesh&& SetVertexData(const void* data, size_t byte_size, uint32_t vertex_count) && {
        if (m_vertex_data) {
            free(m_vertex_data);
            m_vertex_data = nullptr;
        }

        m_vertex_count = vertex_count;

        m_vertex_size = byte_size;
        m_vertex_data = sge::checked_alloc<uint8_t>(byte_size);
        memcpy(m_vertex_data, data, byte_size);

        return std::move(*this);
    }

    [[nodiscard]]
    const sge::Indices& GetIndices() const {
        return m_indices;
    }

    [[nodiscard]]
    const std::vector<sge::MeshAttribute>& GetAttributes() const {
        return m_attributes;
    }

    [[nodiscard]]
    const void* GetVertexData() const {
        return m_vertex_data;
    }

    [[nodiscard]]
    size_t GetVertexSize() const {
        return m_vertex_size;
    }

    [[nodiscard]]
    uint32_t GetVertexCount() const {
        return m_vertex_count;
    }

    [[nodiscard]]
    sge::PrimitiveTopology GetTopology() const {
        return m_topology;
    };

    [[nodiscard]]
    sge::FrontFace GetFrontFace() const {
        return m_front_face;
    };

private:
    Indices m_indices;
    std::vector<MeshAttribute> m_attributes;
    void* m_vertex_data = nullptr;
    size_t m_vertex_size = 0;
    uint32_t m_vertex_count = 0;

    PrimitiveTopology m_topology = PrimitiveTopology::TriangleList;
    FrontFace m_front_face = FrontFace::CCW;
};

} // namespace sge

#endif // SGE_RENDERER_MESH_HPP_