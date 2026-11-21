#ifndef SGE_RENDERER_BUFFER_POOL_HPP_
#define SGE_RENDERER_BUFFER_POOL_HPP_

#include <LLGL/Buffer.h>

#include <SGE/renderer/context.hpp>
#include <SGE/renderer/resource.hpp>

namespace sge {

class BufferPool {
public:
    BufferPool() = default;

    explicit BufferPool(sge::RenderContext& context, const LLGL::BufferDescriptor& desc) : m_desc(desc) {
        if (desc.size > 0) {
            Reserve(context, desc.size);
        }
    }

    bool Reserve(sge::RenderContext& context, const size_t size) {
        if (size <= m_capacity)
            return false;

        const auto stride = m_desc.stride;

        size_t new_capacity = size + size / 2;
        if (stride > 0)
            new_capacity = ((new_capacity + stride - 1) / stride) * stride;

        m_capacity = new_capacity;
        m_desc.size = new_capacity;
        m_buffer = context.CreateUnique(context.GetLLGLContext()->CreateBuffer(m_desc));

        return true;
    }

    [[nodiscard]]
    LLGL::Buffer* Get() const noexcept {
        return m_buffer;
    }

    [[nodiscard]]
    size_t GetStride() const noexcept {
        return m_desc.stride;
    }

private:
    LLGL::BufferDescriptor m_desc = {};
    sge::Unique<LLGL::Buffer> m_buffer = nullptr;
    size_t m_capacity = 0;
};

} // namespace sge

#endif // SGE_RENDERER_BUFFER_POOL_HPP_
