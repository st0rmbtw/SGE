#include <SGE/renderer/camera.hpp>
#include <SGE/engine.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace sge;

void Camera::update_projection_area() {
    switch (m_origin) {

    case CameraOrigin::TopLeft:
        m_area = sge::Rect::from_corners(
            glm::vec2(0.0f),
            glm::vec2(m_viewport) * m_zoom
        );
        m_area_nozoom = sge::Rect::from_corners(
            glm::vec2(0.0f),
            glm::vec2(m_viewport)
        );
    break;

    case CameraOrigin::Center:
        m_area = sge::Rect::from_corners(
            -glm::vec2(m_viewport) / 2.0f * m_zoom,
            glm::vec2(m_viewport) / 2.0f * m_zoom
        );
        m_area_nozoom = sge::Rect::from_corners(
            -glm::vec2(m_viewport) / 2.0f,
            glm::vec2(m_viewport) / 2.0f
        );
    break;
    }
}

void Camera::compute_projection_and_view_matrix() {
    const sge::Rect& projection_area = get_projection_area();
    const sge::Rect& nozoom_projection_area = get_nozoom_projection_area();

    if (sge::Engine::Renderer().Backend().IsOpenGL()) {
        m_projection_matrix = glm::orthoRH_NO(
            projection_area.min.x, projection_area.max.x,
            projection_area.max.y, projection_area.min.y,
            0.0f, 100.0f
        );
        m_nozoom_projection_matrix = glm::orthoRH_NO(
            nozoom_projection_area.min.x, nozoom_projection_area.max.x,
            nozoom_projection_area.max.y, nozoom_projection_area.min.y,
            0.0f, 100.0f
        );
        m_view_matrix = glm::lookAtRH(
            glm::vec3(m_position, 50.0f),
            glm::vec3(m_position, 0.0),
            glm::vec3(0.0, 1.0, 0.0)
        );
    } else {
        m_projection_matrix = glm::orthoLH_ZO(
            projection_area.min.x, projection_area.max.x,
            projection_area.max.y, projection_area.min.y,
            0.0f, 100.0f
        );
        m_nozoom_projection_matrix = glm::orthoLH_ZO(
            nozoom_projection_area.min.x, nozoom_projection_area.max.x,
            nozoom_projection_area.max.y, nozoom_projection_area.min.y,
            0.0f, 100.0f
        );
        m_view_matrix = glm::lookAtLH(
            glm::vec3(m_position, -50.0f),
            glm::vec3(m_position, 0.0),
            glm::vec3(0.0, 1.0, 0.0)
        );
    }

    m_view_proj_matrix = m_projection_matrix * m_view_matrix;
    m_nozoom_view_proj_matrix = m_nozoom_projection_matrix * m_view_matrix;
    m_inv_view_proj_matrix = glm::inverse(m_projection_matrix * m_view_matrix);
}

void Camera::compute_transform_matrix() {
    m_transform_matrix = glm::translate(glm::mat4(1.0), glm::vec3(m_position, 0.));
}

inline glm::vec2 project_point(const glm::mat4& mat, const glm::vec2& point) {
    glm::vec4 res = mat[0] * point.x;
    res = mat[1] * point.y + res;
    res = mat[3] + res;
    return res;
}

glm::vec2 Camera::screen_to_world(const glm::vec2& screen_pos) const {
    const glm::vec2 inverted_y = glm::vec2(screen_pos.x, m_viewport.y - screen_pos.y);
    const glm::vec2 ndc = inverted_y * 2.0f / glm::vec2(m_viewport) - glm::vec2(1.0);
    const glm::mat4 ndc_to_world = m_inv_view_proj_matrix;
    const glm::vec2 world_pos = project_point(ndc_to_world, ndc);
    return world_pos;
}