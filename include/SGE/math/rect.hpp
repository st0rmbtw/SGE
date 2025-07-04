#ifndef _SGE_MATH_RECT_HPP_
#define _SGE_MATH_RECT_HPP_

#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "../defines.hpp"

_SGE_BEGIN

template <class T>
struct rect {
private:
    using vec2 = glm::vec<2, T>;
    using Self = rect<T>;

public:
    vec2 min;
    vec2 max;

    constexpr rect() :
        min(static_cast<T>(0)),
        max(static_cast<T>(0)) {}

    template <typename T2>
    constexpr rect(const rect<T2>& r) :
        min(r.min),
        max(r.max) {}

    constexpr rect(vec2 min, vec2 max) :
        min(min),
        max(max) {}

    template <typename T2>
    inline constexpr Self& operator=(const rect<T2>& other) noexcept {
        this->min = other.min;
        this->max = other.max;
        return *this;
    }

    [[nodiscard]]
    inline constexpr static Self uninitialized() noexcept {
        return Self(vec2(std::numeric_limits<T>::max()), vec2(std::numeric_limits<T>::min()));
    }

    [[nodiscard]]
    inline constexpr static Self from_corners(vec2 p1, vec2 p2) noexcept {
        return Self(glm::min(p1, p2), glm::max(p1, p2));
    }

    [[nodiscard]]
    inline constexpr static Self from_top_left(vec2 origin, vec2 size) noexcept {
        return Self(origin, origin + size);
    }

    [[nodiscard]]
    inline constexpr static Self from_center_size(vec2 origin, vec2 size) noexcept {
        const vec2 half_size = size / static_cast<T>(2);
        return Self::from_center_half_size(origin, half_size);
    }

    [[nodiscard]]
    inline constexpr static Self from_center_half_size(vec2 origin, vec2 half_size) noexcept {
        return Self(origin - half_size, origin + half_size);
    }

    [[nodiscard]]
    inline constexpr Self merge(Self other) {
        return Self(glm::min(min, other.min), glm::max(max, other.max));
    }

    [[nodiscard]]
    inline constexpr T width() const noexcept { return this->max.x - this->min.x; }

    [[nodiscard]]
    inline constexpr T height() const noexcept { return this->max.y - this->min.y; }

    [[nodiscard]]
    inline constexpr T half_width() const noexcept { return this->width() / static_cast<T>(2); }

    [[nodiscard]]
    inline constexpr T half_height() const noexcept { return this->height() / static_cast<T>(2); }

    [[nodiscard]]
    inline constexpr vec2 center() const noexcept { return (this->min + this->max) / static_cast<T>(2); }

    [[nodiscard]]
    inline constexpr vec2 size() const noexcept { return vec2(this->width(), this->height()); }

    [[nodiscard]]
    inline constexpr vec2 half_size() const noexcept { return vec2(this->half_width(), this->half_height()); }

    [[nodiscard]]
    inline constexpr T left() const noexcept { return this->min.x; }

    [[nodiscard]]
    inline constexpr T right() const noexcept { return this->max.x; }

    [[nodiscard]]
    inline constexpr T bottom() const noexcept { return this->min.y; }

    [[nodiscard]]
    inline constexpr T top() const noexcept { return this->max.y; }

    [[nodiscard]]
    inline constexpr rect<T> clamp(vec2 min, vec2 max) const {
        return rect::from_corners(glm::max(this->min, min), glm::min(this->max, max));
    }

    [[nodiscard]]
    inline constexpr rect<T> clamp(const rect<T>& rect) const {
        return rect::from_corners(glm::max(this->min, rect.min), glm::min(this->max, rect.max));
    }

    [[nodiscard]]
    inline constexpr bool contains(const vec2& point) const noexcept {
        return (
            point.x >= this->min.x &&
            point.y >= this->min.y &&
            point.x <= this->max.x &&
            point.y <= this->max.y
        );
    }

    [[nodiscard]]
    inline constexpr bool intersects(const rect& other) const noexcept {
        return (
            this->left() < other.right() &&
            this->right() > other.left() &&
            this->top() > other.bottom() &&
            this->bottom() < other.top()
        );
    }

    [[nodiscard]]
    inline constexpr Self inset(const T l) const noexcept {
        return from_corners(this->min - l, this->max + l);
    }

    inline constexpr Self operator/(const Self &rhs) const noexcept {
        return from_corners(this->min * rhs.min, this->max * rhs.max);
    }

    inline constexpr Self operator/(const T rhs) const noexcept {
        return from_corners(this->min / rhs, this->max / rhs);
    }

    inline constexpr Self operator*(const Self &rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr Self operator*(const T rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr Self operator+(const Self &rhs) const noexcept {
        return from_corners(this->min + rhs.min, this->max + rhs.max);
    }

    inline constexpr Self operator+(const T rhs) const noexcept {
        return from_corners(this->min + rhs, this->max + rhs);
    }

    inline constexpr Self operator-(const Self &rhs) const noexcept {
        return from_corners(this->min - rhs.min, this->max - rhs.max);
    }

    inline constexpr Self operator-(const T rhs) const noexcept {
        return from_corners(this->min - rhs, this->max - rhs);
    }

    inline constexpr Self operator-(const vec2& rhs) const noexcept {
        return from_corners(this->min - rhs, this->max - rhs);
    }

    inline constexpr Self operator/(const vec2& rhs) const noexcept {
        return from_corners(this->min / rhs, this->max / rhs);
    }

    inline constexpr Self operator*(const vec2& rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr Self operator+(const vec2& rhs) const noexcept {
        return from_corners(this->min + rhs, this->max + rhs);
    }

    inline constexpr bool operator==(const Self& rhs) const noexcept {
        return this->min == rhs.min && this->max == rhs.max;
    }
};

using Rect = rect<glm::float32>;
using URect = rect<glm::uint32>;
using IRect = rect<glm::int32>;

_SGE_END

#endif