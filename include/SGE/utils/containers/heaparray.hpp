#ifndef SGE_UTILS_CONTAINERS_HEAP_ARRAY_HPP_
#define SGE_UTILS_CONTAINERS_HEAP_ARRAY_HPP_

#include <cstddef>
#include <utility>

namespace sge {

template <typename T>
class HeapArray {
public:
    HeapArray() = default;

    explicit HeapArray(std::size_t size) : m_size(size) {
        m_data = new T[size];
    }

    ~HeapArray() {
        m_size = 0;
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
    }

    HeapArray(HeapArray<T>&& other) noexcept {
        operator=(std::move(other));
    }

    HeapArray(const HeapArray<T>& other) {
        operator=(other);
    }

    HeapArray& operator=(HeapArray&& other) noexcept {
        m_data = other.m_data;
        m_size = other.m_size;

        other.m_data = nullptr;
        other.m_size = 0;

        return *this;
    }

    HeapArray& operator=(const HeapArray<T>& other) {
        if (this == &other)
            return *this;

        m_data = new T[other.m_size];
        m_size = other.m_size;

        memcpy(m_data, other.m_data, other.m_size * sizeof(T));

        return *this;
    }

    T& operator[](std::size_t i) noexcept {
        return m_data[i];
    }

    const T& operator[](std::size_t i) const noexcept {
        return m_data[i];
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return m_size;
    }

    [[nodiscard]]
    T* data() const noexcept {
        return m_data;
    }

private:
    T* m_data = nullptr;
    std::size_t m_size = 0;
};

} // namespace sge

#endif