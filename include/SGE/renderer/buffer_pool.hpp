#ifndef SGE_RENDERER_BUFFER_POOL_HPP_
#define SGE_RENDERER_BUFFER_POOL_HPP_

#include <utility>

#include <LLGL/Buffer.h>

#include <SGE/renderer/context.hpp>
#include <SGE/renderer/resource.hpp>

namespace sge {

class VertexBufferPool {
public:
    explicit VertexBufferPool() = default;

    explicit VertexBufferPool(LLGL::VertexFormat vertexFormat) :
        m_vertex_format(std::move(vertexFormat))
    {
    }

    explicit VertexBufferPool(sge::RenderContext& context, LLGL::VertexFormat vertexFormat, size_t size)
        : m_vertex_format(std::move(vertexFormat))
    {
        Reserve(context, size);
    }

    bool Reserve(sge::RenderContext& context, size_t size) {
        if (size >= m_capacity) {
            const size_t stride = m_vertex_format.GetStride();
            const size_t sizeBytes = ((size + stride - 1) / stride) * stride;
            m_buffer = context.CreateVertexBuffer(sizeBytes, m_vertex_format);
            m_capacity = sizeBytes;
            return true;
        }
        return false;
    }

    [[nodiscard]]
    LLGL::Buffer* Get() const {
        return m_buffer;    
    }
    
private:
    LLGL::VertexFormat m_vertex_format;
    sge::Unique<LLGL::Buffer> m_buffer = nullptr;
    size_t m_capacity = 0;
};

} // namespace sge

#endif // SGE_RENDERER_BUFFER_POOL_HPP_