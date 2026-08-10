#ifndef SGE_RENDERER_ATTRIBUTES_HPP_
#define SGE_RENDERER_ATTRIBUTES_HPP_

#include <cstdint>
#include <span>
#include <utility>

#include <LLGL/Container/StringLiteral.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/VertexAttribute.h>

#include <SGE/renderer/vertex_format.hpp>
#include <SGE/types/backend.hpp>

namespace sge {

struct Attribute {
    enum class Type : uint8_t {
        PerVertex = 0,
        PerInstance
    };

    Attribute(Type type, sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot);

    static Attribute Vertex(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
        return Attribute(Type::PerVertex, format, std::move(name), std::move(semantic_name), slot);
    }

    static Attribute Instance(sge::VertexFormat format, LLGL::StringLiteral name, LLGL::StringLiteral semantic_name, uint32_t slot = 0) noexcept {
        return Attribute(Type::PerInstance, format, std::move(name), std::move(semantic_name), slot);
    }

    LLGL::StringLiteral name;
    LLGL::StringLiteral semanticName;

    sge::VertexFormat format;
    LLGL::DataType dataType = LLGL::DataType::Undefined;
    uint32_t size = 0;
    uint32_t slot = 0;
    Type type = Type::PerVertex;
};

LLGL::VertexFormat VertexAttributes(sge::RenderBackend backend, uint8_t startLocation, std::span<const Attribute> attributes);

inline LLGL::VertexFormat VertexAttributes(sge::RenderBackend backend, std::span<const Attribute> attributes) {
    return VertexAttributes(backend, 0, attributes);
}

inline LLGL::VertexFormat VertexAttributes(sge::RenderBackend backend, uint8_t startLocation, std::initializer_list<Attribute> attributes) {
    return VertexAttributes(backend, startLocation, { attributes.begin(), attributes.end() });
}

inline LLGL::VertexFormat VertexAttributes(sge::RenderBackend backend, std::initializer_list<Attribute> attributes) {
    return VertexAttributes(backend, 0, { attributes.begin(), attributes.end() });
}

} // namespace sge

#endif // SGE_RENDERER_ATTRIBUTES_HPP_