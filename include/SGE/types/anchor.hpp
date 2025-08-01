#ifndef _SGE_TYPES_ANCHOR_HPP_
#define _SGE_TYPES_ANCHOR_HPP_

#pragma once

#include <cstdint>

#include <SGE/assert.hpp>
#include <glm/vec2.hpp>

#include "../defines.hpp"

_SGE_BEGIN

class Anchor {
public:
    enum Type : uint8_t {
        Center = 0,
        TopLeft,
        TopRight,
        TopCenter,
        BottomLeft,
        BottomRight,
        BottomCenter,
        CenterLeft,
        CenterRight
    };

    constexpr Anchor() = default;
    constexpr Anchor(Type backend) : m_value(backend) {}

    constexpr operator Type() const { return m_value; }
    explicit operator bool() const = delete;

    [[nodiscard]]
    inline glm::vec2 operator*(const glm::vec2& vec) const noexcept {
        return to_vec2() * vec;
    }

    [[nodiscard]]
    constexpr inline glm::vec2 to_vec2() const noexcept {
        switch (m_value) {
        case Anchor::Center:       return {0.5f, 0.5f};
        case Anchor::TopLeft:      return {0.0f, 0.0f};
        case Anchor::TopRight:     return {1.0f, 0.0f};
        case Anchor::BottomLeft:   return {0.0f, 1.0f};
        case Anchor::BottomRight:  return {1.0f, 1.0f};
        case Anchor::TopCenter:    return {0.5f, 0.0f};
        case Anchor::BottomCenter: return {0.5f, 1.0f};
        case Anchor::CenterLeft:   return {0.0f, 0.5f};
        case Anchor::CenterRight:  return {1.0f, 0.5f};
        default: SGE_UNREACHABLE();
        }
    }

private:
    Type m_value = Type::Center;
};

_SGE_END

#endif