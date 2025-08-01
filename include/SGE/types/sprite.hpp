#ifndef _SGE_TYPES_SPRITE_HPP_
#define _SGE_TYPES_SPRITE_HPP_

#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../math/rect.hpp"
#include "anchor.hpp"
#include "texture.hpp"
#include "texture_atlas.hpp"
#include "color.hpp"

#include "../defines.hpp"

_SGE_BEGIN

class BaseSprite {
protected:
    BaseSprite() = default;
    ~BaseSprite() = default;

    explicit BaseSprite(glm::vec2 position) : m_position(position) {}

    BaseSprite(glm::vec2 position, glm::vec2 scale) : 
        m_position(position),
        m_scale(scale) {}

    BaseSprite(glm::vec2 position, glm::vec2 scale, sge::LinearRgba color, Anchor anchor) : 
        m_position(position),
        m_scale(scale),
        m_color(color),
        m_anchor(anchor) {}

public:

    [[nodiscard]]
    inline const glm::vec2& position() const noexcept {
        return m_position;
    }

    [[nodiscard]]
    inline const glm::quat& rotation() const noexcept {
        return m_rotation;
    }

    [[nodiscard]]
    inline const glm::vec2& scale() const noexcept {
        return m_scale;
    }

    [[nodiscard]]
    inline const sge::LinearRgba& color() const noexcept {
        return m_color;
    }

    [[nodiscard]]
    inline const sge::LinearRgba& outline_color() const noexcept {
        return m_outline_color;
    }

    [[nodiscard]]
    inline float outline_thickness() const noexcept {
        return m_outline_thickness;
    }

    [[nodiscard]]
    inline float z() const noexcept {
        return m_z;
    }

    [[nodiscard]]
    inline Anchor anchor() const noexcept {
        return m_anchor;
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
    inline bool ignore_camera_zoom() const noexcept {
        return m_ignore_camera_zoom;
    }

    [[nodiscard]] virtual inline glm::vec2 size() const = 0;

    [[nodiscard]] inline sge::Rect calculate_aabb() const noexcept {
        return sge::Rect::from_top_left(m_position - m_anchor.to_vec2() * size(), size());
    }

    inline BaseSprite& set_position(const glm::vec2& position) noexcept {
        m_position = position;
        return *this;
    }

    inline BaseSprite& set_rotation(const glm::quat& rotation) noexcept {
        m_rotation = rotation;
        return *this;
    }

    inline BaseSprite& set_scale(const glm::vec2& scale) noexcept {
        m_scale = scale;
        return *this;
    }

    inline BaseSprite& set_scale(const float scale) noexcept {
        m_scale = glm::vec2(scale);
        return *this;
    }

    inline BaseSprite& set_custom_size(const std::optional<glm::vec2> size) noexcept {
        m_custom_size = size;
        return *this;
    }

    inline BaseSprite& set_color(const sge::LinearRgba& color) noexcept {
        m_color = color;
        return *this;
    }

    inline BaseSprite& set_outline_color(const sge::LinearRgba& color) noexcept {
        m_outline_color = color;
        return *this;
    }

    inline BaseSprite& set_outline_thickness(const float thickness) noexcept {
        m_outline_thickness = thickness;
        return *this;
    }

    inline BaseSprite& set_anchor(Anchor anchor) noexcept {
        m_anchor = anchor;
        return *this;
    }

    inline BaseSprite& set_flip_x(bool flip_x) noexcept {
        m_flip_x = flip_x;
        return *this;
    }

    inline BaseSprite& set_flip_y(bool flip_y) noexcept {
        m_flip_y = flip_y;
        return *this;
    }

    inline BaseSprite& set_ignore_camera_zoom(bool ignore) noexcept {
        m_ignore_camera_zoom = ignore;
        return *this;
    }

    inline BaseSprite& set_z(float z) noexcept {
        m_z = z;
        return *this;
    }

protected:
    glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec2 m_position = glm::vec2(0.0f);
    glm::vec2 m_scale = glm::vec2(1.0f);
    sge::LinearRgba m_color = sge::LinearRgba(1.0f);
    sge::LinearRgba m_outline_color = sge::LinearRgba(0.0f);
    std::optional<glm::vec2> m_custom_size = std::nullopt;
    float m_outline_thickness = 0.0f;
    float m_z = 1.0f;
    Anchor m_anchor = Anchor::Center;
    bool m_flip_x = false;
    bool m_flip_y = false;
    bool m_ignore_camera_zoom = false;
};

class Sprite : public BaseSprite {
public:
    Sprite() = default;

    Sprite(Texture texture) : BaseSprite(), m_texture(texture) {};

    Sprite(Texture texture, glm::vec2 position) : BaseSprite(position), m_texture(texture) {}
    Sprite(Texture texture, glm::vec2 position, glm::vec2 scale) : BaseSprite(position, scale), m_texture(texture) {}

    inline Sprite& set_texture(Texture texture) { m_texture = texture; return *this; }
    [[nodiscard]] inline const Texture& texture() const { return m_texture; }

    [[nodiscard]]
    inline glm::vec2 size() const override {
        return m_custom_size.value_or(m_texture.size()) * scale();
    }

private:
    Texture m_texture;
};

class TextureAtlasSprite : public BaseSprite {
public:
    TextureAtlasSprite() = default;

    explicit TextureAtlasSprite(TextureAtlas texture_atlas) :
        m_texture_atlas(std::move(texture_atlas)) {}

    TextureAtlasSprite(const TextureAtlas& texture_atlas, glm::vec2 position, glm::vec2 scale) : BaseSprite(position, scale)
    {
        m_texture_atlas = texture_atlas;
    }

    inline void set_index(uint32_t index) { m_index = index; }
    inline void set_index(uint16_t x, uint16_t y) { m_index = y * m_texture_atlas.columns() + x; }

    [[nodiscard]] inline uint32_t index() const { return m_index; }
    [[nodiscard]] inline const TextureAtlas& atlas() const { return m_texture_atlas; }

    [[nodiscard]] glm::vec2 size() const override {
        return m_custom_size.value_or(m_texture_atlas.size()) * scale();
    }

private:
    TextureAtlas m_texture_atlas;
    uint32_t m_index = 0;
};

_SGE_END

#endif