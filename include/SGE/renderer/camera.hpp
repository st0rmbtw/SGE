#ifndef _SGE_RENDERER_CAMERA_HPP_
#define _SGE_RENDERER_CAMERA_HPP_

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../math/rect.hpp"

#include "../defines.hpp"

_SGE_BEGIN

enum class CameraOrigin : uint8_t {
    TopLeft = 0,
    Center = 1
};

enum class CoordinateDirectionX : uint8_t {
    Positive,
    Negative,
};

enum class CoordinateDirectionY : uint8_t {
    Positive,
    Negative,
};

enum class CoordinateDirectionZ : uint8_t {
    Positive,
    Negative,
};

struct CoordinateSystem {
    CoordinateDirectionX right = CoordinateDirectionX::Positive;
    CoordinateDirectionY up = CoordinateDirectionY::Positive;
    CoordinateDirectionZ forward = CoordinateDirectionZ::Positive;
};

class Camera {
public:
    Camera(CameraOrigin origin = CameraOrigin::Center, CoordinateSystem coordinate_system = {}) :
        m_projection_matrix(),
        m_screen_projection_matrix(),
        m_nozoom_projection_matrix(),
        m_view_matrix(),
        m_transform_matrix(),
        m_viewport(0),
        m_position(0.0f),
        m_origin(origin)
    {
        set_coordinate_system(coordinate_system);
    }

    explicit Camera(glm::uvec2 viewport, CameraOrigin origin = CameraOrigin::Center, CoordinateSystem coordinate_system = {}) :
        m_projection_matrix(),
        m_screen_projection_matrix(),
        m_nozoom_projection_matrix(),
        m_view_matrix(),
        m_transform_matrix(),
        m_viewport(viewport),
        m_position(0.0f),
        m_origin(origin)
    {
        set_coordinate_system(coordinate_system);
        update_projection_area();
        compute_projection_and_view_matrix();
    }

    inline void update() {
        compute_projection_and_view_matrix();
        compute_transform_matrix();
    }

    inline void set_position(const glm::vec2& position) { m_position = position; }

    inline void set_zoom(float zoom) {
        m_zoom = zoom;
        update_projection_area();
    }

    inline void set_viewport(const glm::uvec2& viewport) {
        m_viewport = viewport;
        m_screen_projection_matrix = glm::ortho(0.0f, static_cast<float>(viewport.x), static_cast<float>(viewport.y), 0.0f);
        update_projection_area();
    }

    inline void set_flip_horizontal(bool flip_horizontal) {
        m_flip_horizontal = flip_horizontal;
        update_projection_area();
    }

    inline void set_flip_vertical(bool flip_vertical) {
        m_flip_vertical = flip_vertical;
        update_projection_area();
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
    inline auto get_nonscale_projection_matrix() const -> const glm::mat4x4& { return m_nozoom_projection_matrix; }

    [[nodiscard]]
    inline auto get_nonscale_view_projection_matrix() const -> const glm::mat4x4& { return m_nozoom_view_proj_matrix; }

    [[nodiscard]]
    inline auto get_view_matrix() const -> const glm::mat4x4& { return m_view_matrix; }
    
    [[nodiscard]]
    inline auto get_transform_matrix() const -> const glm::mat4x4& { return m_transform_matrix; }

    [[nodiscard]]
    inline auto get_projection_area() const -> const sge::Rect& { return m_area; }

    [[nodiscard]]
    inline auto get_nozoom_projection_area() const -> const sge::Rect& { return m_area_nozoom; }

    [[nodiscard]]
    inline float zoom() const { return m_zoom; }

    [[nodiscard]]
    inline bool flip_horizontal() const { return m_flip_horizontal; }

    [[nodiscard]]
    inline bool flip_vertical() const { return m_flip_vertical; }

    [[nodiscard]]
    inline float right() const { return m_right; }

    [[nodiscard]]
    inline float up() const { return m_up; }

    [[nodiscard]]
    inline float forward() const { return m_forward; }

    [[nodiscard]]
    inline float left() const { return -m_right; }

    [[nodiscard]]
    inline float down() const { return -m_up; }

    [[nodiscard]]
    inline float backward() const { return -m_forward; }

    [[nodiscard]]
    inline glm::vec2 screen_center() const {
        switch (m_origin) {
        case CameraOrigin::TopLeft: {
            const glm::vec2 half = glm::vec2(viewport()) / 2.0f;
        
            return glm::vec2(
                half.x * m_right,
                half.y * m_up
            );
        } break;
        case CameraOrigin::Center:
            return glm::vec2(0.0f, 0.0f);
        break;
        }
    }

private:
    void compute_projection_and_view_matrix();
    void compute_transform_matrix();
    void update_projection_area();

    void set_coordinate_system(CoordinateSystem coordinate_system) {
        switch (coordinate_system.right) {
        case CoordinateDirectionX::Positive:
            m_right = 1.0f;
        break;
        case CoordinateDirectionX::Negative:
            m_right = -1.0f;
        break;
        }

        switch (coordinate_system.up) {
        case CoordinateDirectionY::Positive:
            m_up = 1.0f;
        break;
        case CoordinateDirectionY::Negative:
            m_up = -1.0f;
        break;
        }

        switch (coordinate_system.forward) {
        case CoordinateDirectionZ::Positive:
            m_forward = 1.0f;
        break;
        case CoordinateDirectionZ::Negative:
            m_forward = -1.0f;
        break;
        }
    }

private:
    glm::mat4x4 m_projection_matrix;
    glm::mat4x4 m_screen_projection_matrix;
    glm::mat4x4 m_nozoom_projection_matrix;
    glm::mat4x4 m_view_matrix;
    glm::mat4x4 m_transform_matrix;
    glm::mat4x4 m_inv_view_proj_matrix;
    glm::mat4x4 m_view_proj_matrix;
    glm::mat4x4 m_nozoom_view_proj_matrix;

    sge::Rect m_area;
    sge::Rect m_area_nozoom;

    float m_right;
    float m_up;
    float m_forward;

    glm::uvec2 m_viewport;
    glm::vec2 m_position;
    
    float m_zoom = 1.0f;

    CameraOrigin m_origin;

    bool m_flip_horizontal = false;
    bool m_flip_vertical = false;
};

_SGE_END

#endif