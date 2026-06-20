#include <LLGL/Format.h>

#include <SGE/renderer/framebuffer_pool.hpp>
#include <SGE/types/framebuffer.hpp>

void sge::TemporaryFramebuffer::Release() {
    if (m_pool && m_entry) {
        m_entry->occupied = false;
    }
}