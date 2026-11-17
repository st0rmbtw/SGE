#ifndef SGE_UTILS_FRAME_ARENA_HPP_
#define SGE_UTILS_FRAME_ARENA_HPP_

#include <cstddef>

namespace sge {

namespace FrameArena {
    void Init(size_t size);
    void* Allocate(size_t size, size_t alignment);
    void* Copy(const void* data, size_t dataSize);
    void Clear();

    template <typename T>
    T* Allocate() {
        return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
    }

    template <typename T>
    T* Copy(const T& data) {
        return static_cast<T*>(Copy(&data, sizeof(T)));
    }
};

}

#endif // SGE_UTILS_FRAME_ARENA_HPP_