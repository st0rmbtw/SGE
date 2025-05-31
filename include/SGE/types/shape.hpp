#pragma once

#ifndef _SGE_TYPES_SHAPE_HPP_
#define _SGE_TYPES_SHAPE_HPP_

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "../defines.hpp"

#include "color.hpp"
#include "anchor.hpp"

_SGE_BEGIN

namespace Shape {
    using Type = uint8_t;

    enum : Type {
        Rect = 0,
        Circle,
        Arc
    };
}

struct ShapeCircle {
    float radius = 0.0f;
    LinearRgba color = LinearRgba::white();
    float border_thickness = 0.0f;
    LinearRgba border_color = sge::LinearRgba::transparent();
    Anchor anchor = sge::Anchor::Center;
};

struct ShapeRect {
    glm::vec2 size = glm::vec2(0.0f);
    LinearRgba color = LinearRgba::white();
    float border_thickness = 0.0f;
    LinearRgba border_color = sge::LinearRgba::transparent();
    // [topLeft, topRight, bottomLeft, bottomRight]
    glm::vec4 border_radius = glm::vec4(0.0f);
    Anchor anchor = sge::Anchor::Center;
};

struct ShapeArc {
    float outer_radius;
    float inner_radius;
    /// In radians
    float start_angle;
    /// In radians
    float end_angle;
    sge::LinearRgba color;
    Anchor anchor = sge::Anchor::Center;
};

struct ShapeLine {

};

_SGE_END

#endif