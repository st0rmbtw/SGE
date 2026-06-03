#include <SGE/renderer/context.hpp>
#include <SGE/renderer/resource.hpp>

sge::LLGLResourceRC::~LLGLResourceRC() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
        m_data = nullptr;
    }
}

void sge::LLGLResource::Destroy() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
        m_data = nullptr;
    }
}