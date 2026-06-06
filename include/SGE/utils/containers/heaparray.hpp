#ifndef SGE_UTILS_CONTAINERS_HEAP_ARRAY_HPP_
#define SGE_UTILS_CONTAINERS_HEAP_ARRAY_HPP_

#include <SGE/utils/alloc.hpp>

#include <cstddef>
#include <utility>

namespace sge {

/**
 * @brief A wrapper for C-array (`T*`) with RAII.
 * 
 * @tparam T The element type.
 */
template <typename T>
class HeapArray {
public:
    using iterator = T*;
    using const_iterator = const T*;

public:
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");

    HeapArray() = default;

    explicit HeapArray(std::size_t count) :
        m_data{ sge::checked_alloc<T>(count) },
        m_count{ count }
    {}

    explicit HeapArray(std::size_t count, const T& value) : HeapArray(count) {
        std::fill_n(m_data, count, value);
    }

    HeapArray(const HeapArray& other) {
        if (this == &other)
            return *this;

        const std::size_t size = other.m_count * sizeof(T);
        m_data = new T[size];
        m_count = other.m_count;
        std::memcpy(m_data, other.m_data, size);
    }

    HeapArray(HeapArray&& other) noexcept {
        operator=(std::move(other));
    }

    HeapArray& operator=(HeapArray&& other) noexcept {
        m_data = other.m_data;
        m_count = other.m_count;
        other.m_data = nullptr;
        other.m_count = 0;
        return *this;
    }

    /**
     * @brief Reallocates the array with the new size and moves the elements.
     * 
     * @param new_count New size of the array.
     */
    void resize(std::size_t new_count) {
        if (new_count > 0) {
            T* new_data = new T[new_count * sizeof(T)];
            if (m_data != nullptr) {
                memcpy(new_data, m_data, std::min(m_count, new_count) * sizeof(T));
            }
            delete[] m_data;
            m_data = new_data;
        } else {
            delete[] m_data;
            m_data = nullptr;
        }

        m_count = new_count;
    }

    /**
     * @brief Returns the number of elements in the array.
     */
    [[nodiscard]]
    std::size_t count() const noexcept {
        return m_count;
    }

    /**
     * @brief Returns the size of the array in bytes.
     */
    [[nodiscard]]
    std::size_t size() const noexcept {
        return m_count * sizeof(T);
    }

    /**
     * @brief Returns the pointer to the underlying array.
     */
    [[nodiscard]]
    T* data() noexcept {
        return m_data;
    }

    /**
     * @brief Returns the pointer to the underlying array.
     */
    [[nodiscard]]
    const T* data() const noexcept {
        return m_data;
    }

    /**
     * @brief Returns the iterator to the first element of the array.
     */
    iterator begin() noexcept {
        return m_data;
    }

    /**
     * @brief Returns the iterator to the element following the last element of the array.
     */
    iterator end() noexcept {
        return m_data + m_count;
    }

    /**
     * @brief Returns the iterator to the first element of the array.
     */
    const_iterator cbegin() noexcept {
        return m_data;
    }

    /**
     * @brief Returns the iterator to the element following the last element of the array.
     */
    const_iterator cend() noexcept {
        return m_data + m_count;
    }

    /**
     * @brief Returns a reference to the element at specified location.
     * 
     * @param i Position of the element to return.
     * @return Reference to the requested element.
     */
    [[nodiscard]]
    inline T& operator[](std::size_t i) noexcept {
        return m_data[i];
    }

    /**
     * @brief Returns a reference to the element at specified location.
     * 
     * @param i Position of the element to return.
     * @return Reference to the requested element.
     */
    [[nodiscard]]
    inline const T& operator[](std::size_t i) const noexcept {
        return m_data[i];
    }

    ~HeapArray() {
        if (m_data != nullptr) {
            free(m_data);
            m_data = nullptr;
        }
        m_count = 0;
    }

private:
    T* m_data = nullptr;
    std::size_t m_count = 0;
};

} // namespace sge

#endif