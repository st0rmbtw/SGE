#ifndef CONTAINERS_SWAPBACK_VECTOR_
#define CONTAINERS_SWAPBACK_VECTOR_

#include <utility>
#include <vector>

#include <SGE/defines.hpp>

_SGE_BEGIN

/// A wrapper around std::vector that makes removing elements to be O(1) at the cost of not preserving the order.
template <typename T, typename Alloc = std::allocator<T>>
class SwapbackVector {
public:
    using value_type = std::vector<T>::value_type;
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using reference = std::vector<T>::reference;
    using const_reference = std::vector<T>::const_reference;
    using size_type = std::vector<T>::size_type;

    SwapbackVector() = default;

    void reserve(size_type n) {
        m_vector.reserve(n);
    }

    void push_back(value_type&& x) {
        m_vector.push_back(std::move(x));
    }

    void push_back(const value_type& x) {
        m_vector.push_back(x);
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        m_vector.emplace_back(std::forward<Args>(args)...);
    }

    void erase(size_type index) {
        std::swap(m_vector[index], back());
        m_vector.pop_back();
    }

    [[nodiscard]]
    size_type size() const noexcept {
        return m_vector.size();
    }

    [[nodiscard]]
    size_type capacity() const noexcept {
        return m_vector.capacity();
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return m_vector.empty();
    }

    [[nodiscard]]
    const_reference back() const noexcept {
        return m_vector.back();
    }

    [[nodiscard]]
    reference back() noexcept {
        return m_vector.back();
    }

    [[nodiscard]]
    const_iterator cbegin() const noexcept {
        return m_vector.cbegin();
    }

    [[nodiscard]]
    const_iterator cend() const noexcept {
        return m_vector.cend();
    }

    [[nodiscard]]
    const_iterator begin() const noexcept {
        return m_vector.begin();
    }

    [[nodiscard]]
    const_iterator end() const noexcept {
        return m_vector.end();
    }

    [[nodiscard]]
    iterator begin() noexcept {
        return m_vector.begin();
    }

    [[nodiscard]]
    iterator end() noexcept {
        return m_vector.end();
    }

    value_type& operator[](size_t index) noexcept {
        return m_vector[index];
    }

    const value_type& operator[](size_t index) const noexcept {
        return m_vector[index];
    }

private:
    std::vector<T, Alloc> m_vector;
};

_SGE_END

#endif