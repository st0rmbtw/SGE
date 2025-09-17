#ifndef _SGE_UTILS_BITFLAGS_HPP_
#define _SGE_UTILS_BITFLAGS_HPP_

#include <type_traits>
#include <initializer_list>

namespace sge {

template <typename T>
class BitFlags {
    static_assert(std::is_enum_v<T>, "BitFlags can only be specialized for enum types");
    using UnderlyingT = std::underlying_type_t<T>;

public:
    constexpr BitFlags() = default;
    constexpr BitFlags(UnderlyingT value) : m_data(value) {}
    constexpr BitFlags(T value) : m_data(underlying(value)) {}

    constexpr BitFlags(const std::initializer_list<T> values) noexcept {
        for (const T e : values) {
            m_data |= underlying(e);
        }
    }

    inline constexpr void set(T e, bool set = true) noexcept {
        if (set)
            m_data |= underlying(e);
        else
            m_data &= ~underlying(e);
    }

    inline constexpr BitFlags& operator|=(const T e) noexcept {
        m_data |= underlying(e);
        return *this;
    }

    inline constexpr void reset() noexcept {
        m_data = static_cast<UnderlyingT>(0);
    }

    [[nodiscard]]
    inline constexpr BitFlags operator&(const T e) const noexcept {
        return BitFlags(m_data & underlying(e));
    }

    [[nodiscard]]
    inline constexpr BitFlags operator|(const T e) const noexcept {
        return BitFlags(m_data | underlying(e));
    }

    [[nodiscard]]
    inline constexpr bool operator[](const T e) const noexcept {
        return (m_data & underlying(e)) == underlying(e);
    }

    [[nodiscard]]
    inline constexpr UnderlyingT data() const noexcept {
        return m_data;
    }

private:
    [[nodiscard]]
    static constexpr UnderlyingT underlying(T e) {
        return static_cast<UnderlyingT>(e);
    }

private:
    UnderlyingT m_data = static_cast<UnderlyingT>(0);
};

}

#endif