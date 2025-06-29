#ifndef _SGE_UTILS_BITFLAGS_HPP_
#define _SGE_UTILS_BITFLAGS_HPP_

#include <type_traits>
#include <initializer_list>

template <typename T>
class BitFlags {
    static_assert(std::is_enum_v<T>, "BitFlags can only be specialized for enum types");
    using UnderlyingT = std::underlying_type_t<T>;

public:
    BitFlags() = default;
    BitFlags(UnderlyingT value) : m_data(value) {}
    BitFlags(T value) : m_data(underlying(value)) {}

    BitFlags(const std::initializer_list<T> values) {
        for (const T e : values) {
            m_data |= underlying(e);
        }
    }

    inline constexpr void set(T e, bool set = true) {
        if (set)
            m_data |= underlying(e);
        else
            m_data &= ~underlying(e);
    }

    inline constexpr BitFlags& operator|=(const T e) noexcept {
        m_data |= underlying(e);
        return *this;
    }

    inline constexpr BitFlags operator&(const T e) const noexcept {
        return BitFlags(m_data & underlying(e));
    }

    inline constexpr BitFlags operator|(const T e) const noexcept {
        return BitFlags(m_data | underlying(e));
    }

    inline constexpr bool operator[](const T e) const noexcept {
        return (m_data & underlying(e)) == underlying(e);
    }

    [[nodiscard]]
    inline constexpr UnderlyingT data() const noexcept {
        return m_data;
    }

private:
    static constexpr UnderlyingT underlying(T e) {
        return static_cast<UnderlyingT>(e);
    }

private:
    UnderlyingT m_data = static_cast<UnderlyingT>(0);
};

#endif