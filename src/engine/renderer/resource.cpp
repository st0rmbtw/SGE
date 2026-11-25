#include <SGE/renderer/context.hpp>
#include <SGE/renderer/resource.hpp>

sge::LLGLResourceRC::~LLGLResourceRC() {
    if (m_data && m_render_context) {
        m_render_context->ReleaseUntyped(*m_data);
    }
    m_data = nullptr;
}

void sge::LLGLResource::Destroy() {
    if (m_data && m_render_context) {
        m_render_context->ReleaseUntyped(*m_data);
    }
    m_data = nullptr;
}

void sge::ResourceDeleter::operator()(LLGL::RenderSystemChild* child) const noexcept {
    SGE_ASSERT_M(ctx, "deleter is uninitialized!");
    if (child) {
        ctx->ReleaseUntyped(*child);
    }
}