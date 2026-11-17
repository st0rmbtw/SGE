#include <SGE/utils/frame_arena.hpp>

#include <SGE/utils/containers/arena.hpp>

static sge::Arena g_frame_arena;

void sge::FrameArena::Init(size_t size) {
    g_frame_arena = sge::Arena(size);
}

void* sge::FrameArena::Allocate(size_t size, size_t alignment) {
    return g_frame_arena.allocate(size, alignment);
}

void* sge::FrameArena::Copy(const void* data, size_t dataSize) {
    return g_frame_arena.copy(data, dataSize);
}

void sge::FrameArena::Clear() {
    g_frame_arena.clear();
}