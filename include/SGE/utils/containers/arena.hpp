#ifndef SGE_UTILS_CONTAINERS_ARENA_HPP_
#define SGE_UTILS_CONTAINERS_ARENA_HPP_

#include <cstdint>
#include <memory>

#include <SGE/assert.hpp>

namespace sge {

class Arena {
public:
    Arena() = default;

    explicit Arena(size_t size) :
        m_buffer{ std::make_unique<uint8_t[]>(size) },
        m_capacity{ size }
    {}

    void* allocate(size_t size, size_t alignment) {
        SGE_ASSERT_M(m_buffer != nullptr, "Arena is not initialized!");
        SGE_ASSERT_M(m_capacity > 0, "Arena is not initialized!");

        size_t space = m_capacity - m_offset;
        void* aligned_ptr = m_buffer.get() + m_offset;

        bool alignSuccess = std::align(alignment, size, aligned_ptr, space) != nullptr;
        if (!alignSuccess)
            SGE_PANIC("bad allocation. Arena overflow?");

        m_offset = static_cast<uint8_t*>(aligned_ptr) - m_buffer.get() + size;

        return aligned_ptr;
    }

    void* copy(const void* data, size_t dataSize) {
        void* buffer = allocate(dataSize, 1);
        memcpy(buffer, data, dataSize);
        return buffer;
    }

    template <typename T>
    void* allocate(size_t alignment = alignof(T)) {
        return allocate(sizeof(T), alignment);
    }

    void clear() {
        m_offset = 0;
    }

private:
    std::unique_ptr<uint8_t[]> m_buffer = nullptr;
    size_t m_capacity = 0;
    size_t m_offset = 0;
};

}

#endif // SGE_UTILS_ARENA_HPP_
