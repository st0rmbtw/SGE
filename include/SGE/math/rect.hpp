#ifndef _SGE_MATH_RECT_HPP_
#define _SGE_MATH_RECT_HPP_

#pragma once

#include <glm/glm.hpp>
#include <limits>

namespace sge {

template <class T>
struct rect {
private:
    using vec_t = glm::vec<2, T>;
    using self = rect<T>;

public:
    vec_t min;
    vec_t max;

    constexpr rect() :
        min(static_cast<T>(0)),
        max(static_cast<T>(0)) {}

    template <typename T2>
    explicit constexpr rect(const rect<T2>& r) :
        min(vec_t(r.min)),
        max(vec_t(r.max)) {}

    constexpr rect(vec_t min, vec_t max) :
        min(min),
        max(max) {}

    [[nodiscard]]
    inline constexpr static self uninitialized() noexcept {
        return self(vec_t(std::numeric_limits<T>::max()), vec_t(std::numeric_limits<T>::min()));
    }

    [[nodiscard]]
    inline constexpr static self from_corners(vec_t p1, vec_t p2) noexcept {
        return self(glm::min(p1, p2), glm::max(p1, p2));
    }

    [[nodiscard]]
    inline constexpr static self from_top_left(vec_t origin, vec_t size) noexcept {
        return self(origin, origin + size);
    }

    [[nodiscard]]
    inline constexpr static self from_center_size(vec_t origin, vec_t size) noexcept {
        const vec_t half_size = size / static_cast<T>(2);
        return self::from_center_half_size(origin, half_size);
    }

    [[nodiscard]]
    inline constexpr static self from_center_half_size(vec_t origin, vec_t half_size) noexcept {
        return self(origin - half_size, origin + half_size);
    }

    [[nodiscard]]
    inline constexpr self merge(self other) {
        return self(glm::min(min, other.min), glm::max(max, other.max));
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
    inline constexpr vec_t center() const noexcept { return (this->min + this->max) / static_cast<T>(2); }

    [[nodiscard]]
    inline constexpr vec_t size() const noexcept { return vec_t(this->width(), this->height()); }

    [[nodiscard]]
    inline constexpr vec_t half_size() const noexcept { return vec_t(this->half_width(), this->half_height()); }

    [[nodiscard]]
    inline constexpr T left() const noexcept { return this->min.x; }

    [[nodiscard]]
    inline constexpr T right() const noexcept { return this->max.x; }

    [[nodiscard]]
    inline constexpr T bottom() const noexcept { return this->min.y; }

    [[nodiscard]]
    inline constexpr T top() const noexcept { return this->max.y; }

    [[nodiscard]]
    inline constexpr rect<T> clamp(vec_t min, vec_t max) const {
        return rect::from_corners(glm::max(this->min, min), glm::min(this->max, max));
    }

    [[nodiscard]]
    inline constexpr rect<T> clamp(const rect<T>& rect) const {
        return rect::from_corners(glm::max(this->min, rect.min), glm::min(this->max, rect.max));
    }

    [[nodiscard]]
    inline constexpr bool contains(const vec_t& point) const noexcept {
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
    inline constexpr self inset(const T l) const noexcept {
        return from_corners(this->min - l, this->max + l);
    }

    inline constexpr self operator/(const self &rhs) const noexcept {
        return from_corners(this->min * rhs.min, this->max * rhs.max);
    }

    inline constexpr self operator/(const T rhs) const noexcept {
        return from_corners(this->min / rhs, this->max / rhs);
    }

    inline constexpr self operator*(const self &rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr self operator*(const T rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr self operator+(const self &rhs) const noexcept {
        return from_corners(this->min + rhs.min, this->max + rhs.max);
    }

    inline constexpr self operator+(const T rhs) const noexcept {
        return from_corners(this->min + rhs, this->max + rhs);
    }

    inline constexpr self operator-(const self &rhs) const noexcept {
        return from_corners(this->min - rhs.min, this->max - rhs.max);
    }

    inline constexpr self operator-(const T rhs) const noexcept {
        return from_corners(this->min - rhs, this->max - rhs);
    }

    inline constexpr self operator-(const vec_t& rhs) const noexcept {
        return from_corners(this->min - rhs, this->max - rhs);
    }

    inline constexpr self operator/(const vec_t& rhs) const noexcept {
        return from_corners(this->min / rhs, this->max / rhs);
    }

    inline constexpr self operator*(const vec_t& rhs) const noexcept {
        return from_corners(this->min * rhs, this->max * rhs);
    }

    inline constexpr self operator+(const vec_t& rhs) const noexcept {
        return from_corners(this->min + rhs, this->max + rhs);
    }

    inline constexpr bool operator==(const self& rhs) const noexcept {
        return this->min == rhs.min && this->max == rhs.max;
    }
};

using Rect = rect<glm::float32>;
using URect = rect<glm::uint32>;
using IRect = rect<glm::int32>;

} // namespace sge

#endif