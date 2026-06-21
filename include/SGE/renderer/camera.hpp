#ifndef _SGE_RENDERER_CAMERA_HPP_
#define _SGE_RENDERER_CAMERA_HPP_

#pragma once

#include <SGE/assert.hpp>
#include <SGE/defines.hpp>
#include <SGE/math/rect.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/size.hpp>
#include <SGE/types/transform.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace sge {

enum class CameraOrigin : uint8_t {
    TopLeft = 0,
    Center = 1
};

struct CameraConfig {
    CameraOrigin origin = CameraOrigin::Center;
    uint8_t samples = 1;
};

class Camera {
public:
    Camera() = default;

    explicit Camera(sge::Size viewport, const CameraConfig& config = {}) :
        m_viewport(viewport),
        m_origin(config.origin),
        m_samples(config.samples)
    {
        update_projection_area();
        compute_projection_and_view_matrix();
    }

    explicit Camera(const CameraConfig& config) :
        m_origin(config.origin),
        m_samples(config.samples)
    {
    }

    inline void set_position(glm::vec3 position) noexcept {
        m_view_dirty = true;
        m_transform.translation = position;
    }

    inline void set_position(glm::vec2 position) noexcept {
        m_view_dirty = true;
        m_transform.translation = glm::vec3(position, 0.0f);
    }

    inline void set_zoom(float zoom) noexcept {
        m_zoom = zoom;
        update_projection_area();
    }

    inline void set_viewport(Size viewport) {
        set_viewport(viewport.width, viewport.height);
    }

    inline void set_viewport(uint32_t width, uint32_t height) {
        m_proj_dirty = true;
        m_viewport.width = width;
        m_viewport.height = height;
        m_screen_projection_matrix = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
        update_projection_area();
    }

    inline void set_samples(uint8_t samples) noexcept {
        m_samples = samples;
    }

    [[nodiscard]]
    glm::vec2 screen_to_world(const glm::vec2& screen_pos) const;

    [[nodiscard]]
    inline const Transform& GetTransform() const noexcept {
        return m_transform;
    }

    [[nodiscard]]
    inline Transform& GetTransform() noexcept {
        return m_transform;
    }

    [[nodiscard]]
    inline sge::Size viewport() const noexcept {
        return m_viewport;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_projection_matrix() const noexcept {
        compute_projection_and_view_matrix();
        return m_projection_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_inv_view_projection_matrix() const noexcept {
        compute_projection_and_view_matrix();
        return m_inv_view_proj_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_view_projection_matrix() const noexcept {
        compute_projection_and_view_matrix();
        return m_view_proj_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_screen_projection_matrix() const noexcept {
        return m_screen_projection_matrix;
    }

    [[nodiscard]]
    inline const glm::mat4x4& get_view_matrix() const noexcept {
        compute_projection_and_view_matrix();
        return m_view_matrix;
    }

    [[nodiscard]]
    inline const sge::Rect& get_projection_area() const noexcept {
        return m_area;
    }

    [[nodiscard]]
    inline float zoom() const noexcept {
        return m_zoom;
    }

    [[nodiscard]]
    inline uint8_t samples() const noexcept {
        return m_samples;
    }

    [[nodiscard]]
    inline glm::vec2 screen_center() const noexcept {
        switch (m_origin) {
        case CameraOrigin::TopLeft: {
            const glm::vec2 half = glm::vec2(m_viewport.width, m_viewport.height) / 2.0f;

            return glm::vec2(
                half.x * m_transform.Right().x,
                half.y * m_transform.Up().y
            );
        } break;
        case CameraOrigin::Center:
            return glm::vec2(0.0f, 0.0f);
        break;
        }
    }

private:
    void compute_projection_and_view_matrix() const;
    void update_projection_area() noexcept;

private:
    mutable glm::mat4x4 m_projection_matrix = glm::identity<glm::mat4>();
    mutable glm::mat4x4 m_screen_projection_matrix = glm::identity<glm::mat4>();
    mutable glm::mat4x4 m_view_matrix = glm::identity<glm::mat4>();
    mutable glm::mat4x4 m_inv_view_proj_matrix = glm::identity<glm::mat4>();
    mutable glm::mat4x4 m_view_proj_matrix = glm::identity<glm::mat4>();
    mutable glm::mat4x4 m_last_transform_matrix = glm::identity<glm::mat4>();

    sge::Transform m_transform;

    sge::Rect m_area;

    sge::Size m_viewport = sge::Size(0);

    float m_zoom = 1.0f;

    CameraOrigin m_origin = CameraOrigin::TopLeft;

    uint8_t m_samples = 1;

    mutable bool m_view_dirty = false;
    mutable bool m_proj_dirty = false;
};

} // namespace sge

#endif