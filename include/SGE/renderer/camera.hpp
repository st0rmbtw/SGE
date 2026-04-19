#ifndef _SGE_RENDERER_CAMERA_HPP_
#define _SGE_RENDERER_CAMERA_HPP_

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SGE/assert.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/defines.hpp>
#include <SGE/types/backend.hpp>

namespace sge {

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

struct CameraConfig {
    CameraOrigin origin = CameraOrigin::Center;
    CoordinateSystem coordinateSystem = {};
    uint8_t samples = 1;
};

class Camera {
public:
    Camera() = default;

    explicit Camera(RenderBackend backend, glm::uvec2 viewport, const CameraConfig& config = {}) :
        m_viewport(viewport),
        m_origin(config.origin),
        m_backend(backend),
        m_samples(config.samples),
        m_changed(true)
    {
        set_coordinate_system(config.coordinateSystem);
        update_projection_area();
        compute_projection_and_view_matrix();
    }

    explicit Camera(RenderBackend backend, const CameraConfig& config = {}) :
        m_origin(config.origin),
        m_backend(backend),
        m_samples(config.samples),
        m_changed(true)
    {
        set_coordinate_system(config.coordinateSystem);
    }

    inline void update() {
        if (m_changed) {
            compute_projection_and_view_matrix();
            m_changed = false;
        }
    }

    inline void set_position(const glm::vec2& position) noexcept {
        m_changed = true;
        m_position = position;
    }

    inline void set_zoom(float zoom) noexcept {
        m_changed = true;
        m_zoom = zoom;
        update_projection_area();
    }

    inline void set_viewport(const glm::uvec2& viewport) {
        m_changed = true;
        m_viewport = viewport;
        m_screen_projection_matrix = glm::ortho(0.0f, static_cast<float>(viewport.x), static_cast<float>(viewport.y), 0.0f);
        update_projection_area();
    }

    inline void set_flip_horizontal(bool flip_horizontal) noexcept {
        m_changed = true;
        m_flip_horizontal = flip_horizontal;
        update_projection_area();
    }

    inline void set_flip_vertical(bool flip_vertical) noexcept {
        m_changed = true;
        m_flip_vertical = flip_vertical;
        update_projection_area();
    }

    inline void set_samples(uint8_t samples) noexcept {
        m_samples = samples;
    }

    [[nodiscard]]
    auto screen_to_world(const glm::vec2 &screen_pos) const -> glm::vec2;

    [[nodiscard]]
    inline const glm::vec2& position() const noexcept {
        return m_position;
    }

    [[nodiscard]]
    inline const glm::uvec2& viewport() const noexcept {
        return m_viewport;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_projection_matrix() const noexcept {
        return m_projection_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_inv_view_projection_matrix() const noexcept {
        return m_inv_view_proj_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_view_projection_matrix() const noexcept {
        return m_view_proj_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_screen_projection_matrix() const noexcept {
        return m_screen_projection_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_nonscale_projection_matrix() const noexcept {
        return m_nozoom_projection_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_nonscale_view_projection_matrix() const noexcept {
        return m_nozoom_view_proj_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_view_matrix() const noexcept {
        return m_view_matrix;
    }

    [[nodiscard]]
    inline const sge::Rect& get_projection_area() const noexcept {
        return m_area;
    }

    [[nodiscard]]
    inline const sge::Rect& get_nozoom_projection_area() const noexcept {
        return m_area_nozoom;
    }

    [[nodiscard]]
    inline float zoom() const noexcept {
        return m_zoom;
    }

    [[nodiscard]]
    inline bool changed() const noexcept {
        return m_changed;
    }

    [[nodiscard]]
    inline bool flipped_horizontally() const noexcept {
        return m_flip_horizontal;
    }

    [[nodiscard]]
    inline bool flipped_vertically() const noexcept {
        return m_flip_vertical;
    }

    [[nodiscard]]
    inline uint8_t samples() const noexcept {
        return m_samples;
    }

    [[nodiscard]]
    inline float right() const noexcept {
        return m_right;
    }

    [[nodiscard]]
    inline float up() const noexcept {
        return m_up;
    }

    [[nodiscard]]
    inline float forward() const noexcept {
        return m_forward;
    }

    [[nodiscard]]
    inline float left() const noexcept {
        return -m_right;
    }

    [[nodiscard]]
    inline float down() const noexcept {
        return -m_up;
    }

    [[nodiscard]]
    inline float backward() const noexcept {
        return -m_forward;
    }

    [[nodiscard]]
    inline glm::vec2 screen_center() const noexcept {
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

        default: SGE_UNREACHABLE();
        }
    }

private:
    void compute_projection_and_view_matrix();
    void update_projection_area() noexcept;

    void set_coordinate_system(CoordinateSystem coordinate_system) noexcept {
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
    glm::mat4x4 m_projection_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_screen_projection_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_nozoom_projection_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_view_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_inv_view_proj_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_view_proj_matrix = glm::identity<glm::mat4>();
    glm::mat4x4 m_nozoom_view_proj_matrix = glm::identity<glm::mat4>();

    sge::Rect m_area;
    sge::Rect m_area_nozoom;

    glm::uvec2 m_viewport = glm::uvec2(0);
    glm::vec2 m_position = glm::vec2(0.0f);

    float m_right;
    float m_up;
    float m_forward;

    float m_zoom = 1.0f;

    CameraOrigin m_origin = CameraOrigin::TopLeft;
    RenderBackend m_backend;

    uint8_t m_samples = 1;

    bool m_flip_horizontal = false;
    bool m_flip_vertical = false;
    bool m_changed = false;
};

}

#endif