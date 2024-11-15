#ifndef RENDERER_CAMERA_H
#define RENDERER_CAMERA_H

#pragma once

#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../math/rect.hpp"

class Camera {
public:
    Camera() :
        m_projection_matrix(),
        m_screen_projection_matrix(),
        m_view_matrix(),
        m_transform_matrix(),
        m_viewport(0),
        m_position(0.0f) {}

    explicit Camera(glm::uvec2 viewport) :
        m_projection_matrix(),
        m_screen_projection_matrix(),
        m_view_matrix(),
        m_transform_matrix(),
        m_viewport(std::move(viewport)),
        m_position(0.0f)
    {
        update_projection_area();
        compute_projection_and_view_matrix();
        m_mutated = true;
    }

    inline void update() {
        compute_projection_and_view_matrix();
        compute_transform_matrix();

        m_needs_update = false;
        m_mutated = true;
    }

    inline void set_position(const glm::vec2& position) {
        m_position = position;
        m_needs_update = true;
        m_mutated = true;
    }

    inline void set_zoom(float zoom) {
        m_zoom = zoom;
        update_projection_area();

        m_needs_update = true;
        m_mutated = true;
    }

    inline void set_viewport(const glm::uvec2& viewport) {
        m_viewport = viewport;
        m_screen_projection_matrix = glm::ortho(0.0f, static_cast<float>(viewport.x), static_cast<float>(viewport.y), 0.0f);
        m_area_nonscale = math::Rect::from_corners(
            -glm::vec2(m_viewport) / 2.0f,
            glm::vec2(m_viewport) / 2.0f
        );
        update_projection_area();

        m_needs_update = true;
        m_mutated = true;
    }

    inline void set_mutated(bool mutated) {
        m_mutated = mutated;
    }

    [[nodiscard]]
    auto screen_to_world(const glm::vec2 &screen_pos) const -> glm::vec2;

    [[nodiscard]]
    inline auto position() const -> const glm::vec2& { return m_position; }

    [[nodiscard]]
    inline auto viewport() const -> const glm::uvec2& { return m_viewport; }

    [[nodiscard]]
    inline auto get_projection_matrix() const -> const glm::mat4x4& { return m_projection_matrix; }

    [[nodiscard]]
    inline auto get_inv_view_projection_matrix() const -> const glm::mat4x4& { return m_inv_view_proj_matrix; }

    [[nodiscard]]
    inline auto get_view_projection_matrix() const -> const glm::mat4x4& { return m_view_proj_matrix; }

    [[nodiscard]]
    inline auto get_screen_projection_matrix() const -> const glm::mat4x4& { return m_screen_projection_matrix; }

    [[nodiscard]]
    inline auto get_view_matrix() const -> const glm::mat4x4& { return m_view_matrix; }
    
    [[nodiscard]]
    inline auto get_transform_matrix() const -> const glm::mat4x4& { return m_transform_matrix; }

    [[nodiscard]]
    inline auto get_projection_area() const -> const math::Rect& { return m_area; }

    [[nodiscard]]
    inline float zoom() const { return m_zoom; }

    [[nodiscard]]
    inline bool needs_update() const { return m_needs_update; }

    [[nodiscard]]
    inline bool mutated() const { return m_mutated; }

private:
    void compute_projection_and_view_matrix();
    void compute_transform_matrix();
    void update_projection_area();

private:
    glm::mat4x4 m_projection_matrix;
    glm::mat4x4 m_screen_projection_matrix;
    glm::mat4x4 m_view_matrix;
    glm::mat4x4 m_transform_matrix;
    glm::mat4x4 m_inv_view_proj_matrix;
    glm::mat4x4 m_view_proj_matrix;

    math::Rect m_area;
    math::Rect m_area_nonscale;

    glm::uvec2 m_viewport;
    glm::vec2 m_position;
    
    float m_zoom = 1.0f;
    bool m_needs_update = false;
    bool m_mutated = false;
};

#endif