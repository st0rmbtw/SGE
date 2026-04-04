#pragma once

#ifndef _SGE_TYPES_SHAPE_HPP_
#define _SGE_TYPES_SHAPE_HPP_

#include <cstdint>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "color.hpp"
#include "anchor.hpp"

namespace sge {

namespace Shape {
    using Type = uint8_t;

    enum : Type {
        Rect = 0,
        Circle,
        Arc
    };
}

class BorderRadius {
public:
    enum class Type : uint8_t {
        Absolute = 0,
        Relative = 1
    };

public:
    BorderRadius() = default;

    explicit BorderRadius(Type type, glm::vec4 values) : 
        m_values(values),
        m_type(type) {}

    static inline BorderRadius Relative(glm::vec4 values) {
        return BorderRadius(Type::Relative, values);
    }

    static inline BorderRadius Relative(float top_left, float top_right, float bottom_left, float bottom_right) {
        return BorderRadius(Type::Relative, glm::vec4(top_left, top_right, bottom_left, bottom_right));
    }

    static inline BorderRadius Relative(float all) {
        return BorderRadius(Type::Relative, glm::vec4(all));
    }

    static inline BorderRadius Absolute(glm::vec4 values) {
        return BorderRadius(Type::Absolute, values);
    }

    static inline BorderRadius Absolute(float top_left, float top_right, float bottom_left, float bottom_right) {
        return BorderRadius(Type::Absolute, glm::vec4(top_left, top_right, bottom_left, bottom_right));
    }

    static inline BorderRadius Absolute(float all) {
        return BorderRadius(Type::Absolute, glm::vec4(all));
    }

public:
    [[nodiscard]]
    inline glm::ivec4 values() const {
        return m_values;
    }

    [[nodiscard]]
    inline Type type() const {
        return m_type;
    }

    [[nodiscard]]
    inline bool is_absolute() const {
        return m_type == Type::Absolute;
    }

    [[nodiscard]]
    inline bool is_relative() const {
        return m_type == Type::Relative;
    }

private:
    glm::vec4 m_values = glm::vec4(0);
    Type m_type = Type::Absolute;
};

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
    BorderRadius border_radius{};
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

}

#endif