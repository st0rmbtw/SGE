#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/context.hpp>

sge::LLGLResourceRC::~LLGLResourceRC() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
    }
}

sge::LLGLResource::~LLGLResource() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
    }
}