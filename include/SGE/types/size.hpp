#pragma once

#ifndef SGE_TYPES_SHAPE_HPP_
#define SGE_TYPES_SHAPE_HPP_

#include <cstdint>

#include <LLGL/Types.h>

#include <glm/vec2.hpp>

namespace sge {

struct Size {
    uint32_t width = 0;
    uint32_t height = 0;

    constexpr explicit Size(uint32_t value) : width(value), height(value) {}
    constexpr explicit Size(uint32_t w, uint32_t h) : width(w), height(h) {}

    constexpr Size(glm::uvec2 v) : width(v.x), height(v.y) {}
    constexpr Size(LLGL::Extent2D e) : width(e.width), height(e.height) {}

    operator glm::uvec2() const noexcept {
        return glm::uvec2(width, height);
    }

    operator LLGL::Extent2D() const noexcept {
        return LLGL::Extent2D(width, height);
    }

    explicit operator glm::vec2() const noexcept {
        return glm::vec2(width, height);
    }
};

} // namespace sge



#endif // SGE_TYPES_SHAPE_HPP_
