#ifndef _SGE_TYPES_NINE_PATCH_HPP_
#define _SGE_TYPES_NINE_PATCH_HPP_

#pragma once

#include <glm/gtc/quaternion.hpp>

#include "texture.hpp"
#include "anchor.hpp"
#include "color.hpp"

#include "../math/rect.hpp"
#include "../defines.hpp"

_SGE_BEGIN

class NinePatch {
public:
    NinePatch() = default;

    NinePatch(Texture texture, glm::uvec4 margin) :
        m_texture(texture),
        m_margin(margin),
        m_size(texture.size()) {}

    inline NinePatch& set_texture(const Texture& texture) noexcept {
        m_texture = texture;
        return *this;
    }

    inline NinePatch& set_position(glm::vec2 position) noexcept {
        m_position = position;
        return *this;
    }

    inline NinePatch& set_rotation(glm::quat rotation) noexcept {
        m_rotation = rotation;
        return *this;
    }

    inline NinePatch& set_margin(glm::uvec4 margin) noexcept {
        m_margin = margin;
        return *this;
    }

    inline NinePatch& set_color(sge::LinearRgba color) noexcept {
        m_color = color;
        return *this;
    }

    inline NinePatch& set_scale(glm::vec2 scale) noexcept {
        m_scale = scale;
        return *this;
    }

    inline NinePatch& set_size(glm::vec2 size) noexcept {
        m_size = size;
        return *this;
    }

    inline NinePatch& set_flip_x(bool flip_x) noexcept {
        m_flip_x = flip_x;
        return *this;
    }

    inline NinePatch& set_flip_y(bool flip_y) noexcept {
        m_flip_y = flip_y;
        return *this;
    }

    inline NinePatch& set_anchor(Anchor anchor) noexcept {
        m_anchor = anchor;
        return *this;
    }

    [[nodiscard]]
    inline const Texture& texture() const noexcept {
        return m_texture;
    }

    [[nodiscard]]
    inline glm::vec2 position() const noexcept {
        return m_position;
    }

    [[nodiscard]]
    inline glm::quat rotation() const noexcept {
        return m_rotation;
    }

    [[nodiscard]]
    inline glm::uvec4 margin() const noexcept {
        return m_margin;
    }

    [[nodiscard]]
    inline sge::LinearRgba color() const noexcept {
        return m_color;
    }

    [[nodiscard]]
    inline glm::vec2 scale() const noexcept {
        return m_scale;
    }

    [[nodiscard]]
    inline bool flip_x() const noexcept {
        return m_flip_x;
    }

    [[nodiscard]]
    inline bool flip_y() const noexcept {
        return m_flip_y;
    }

    [[nodiscard]]
    inline Anchor anchor() const noexcept {
        return m_anchor;
    }

    [[nodiscard]]
    inline glm::vec2 size() const noexcept {
        return m_size * scale();
    }

    [[nodiscard]]
    inline sge::Rect calculate_aabb() const noexcept {
        return sge::Rect::from_top_left(m_position - m_anchor.to_vec2() * size(), size());
    }

private:
    Texture m_texture;
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::uvec4 m_margin = glm::uvec4(0); // Left Right Top Bottom
    sge::LinearRgba m_color = sge::LinearRgba::white();
    glm::vec2 m_position = glm::vec2(0.0f);
    glm::vec2 m_size = glm::vec2(1.0f);
    glm::vec2 m_scale = glm::vec2(1.0f);
    bool m_flip_x = false;
    bool m_flip_y = false;
    Anchor m_anchor = Anchor::Center;
};

_SGE_END

#endif