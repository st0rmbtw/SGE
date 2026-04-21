#pragma once

#ifndef SGE_TYPES_SHAPE_HPP_
#define SGE_TYPES_SHAPE_HPP_

#include <cstdint>
#include <glm/vec2.hpp>
#include <LLGL/Types.h>

namespace sge {

struct Size {
    uint32_t width = 0;
    uint32_t height = 0;

    Size(uint32_t value) : width(value), height(value) {}
    Size(uint32_t w, uint32_t h) : width(w), height(h) {}
    Size(glm::uvec2 v) : width(v.x), height(v.y) {}
    Size(LLGL::Extent2D e) : width(e.width), height(e.height) {}

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

}



#endif // SGE_TYPES_SHAPE_HPP_
