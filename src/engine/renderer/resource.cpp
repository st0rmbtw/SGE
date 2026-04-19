#include <SGE/renderer/resource.hpp>
#include <SGE/renderer/context.hpp>

sge::LLGLResourceRC::~LLGLResourceRC() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
        SGE_LOG_DEBUG("Destroyed: {}", m_id);
    }
}

uint32_t sge::LLGLResourceRC::s_id = 0;


sge::LLGLResource::~LLGLResource() {
    if (m_data) {
        m_render_context->ReleaseUntyped(*m_data);
    }
}