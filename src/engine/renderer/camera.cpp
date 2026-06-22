#include <SGE/engine.hpp>
#include <SGE/renderer/camera.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {

inline glm::vec2 project_point(const glm::mat4& mat, const glm::vec2& point) {
    glm::vec4 res = mat[0] * point.x;
    res = mat[1] * point.y + res;
    res = mat[3] + res;
    return res;
}

} // namespace

void sge::Camera::update_projection_area() noexcept {
    const glm::vec2 viewport = glm::vec2(m_viewport);

    switch (m_origin) {
    case CameraOrigin::TopLeft: {
        glm::vec2 min;
        min.x = 0.0f;
        min.y = 0.0f;

        glm::vec2 max;
        max.x = viewport.x;
        max.y = viewport.y;

        m_area = sge::Rect(
            min * m_zoom,
            max * m_zoom
        );
    } break;

    case CameraOrigin::Center: {
        glm::vec2 min;
        min.x = -viewport.x / 2.0f;
        min.y = -viewport.y / 2.0f;

        glm::vec2 max;
        max.x = viewport.x / 2.0f;
        max.y = viewport.y / 2.0f;

        m_area = sge::Rect::from_corners(
            min * m_zoom,
            max * m_zoom
        );
    } break;
    }

    m_proj_dirty = true;
}

void sge::Camera::compute_projection_and_view_matrix() const {
    const glm::mat4 transform_matrix = m_transform.ComputeMatrix();
    if (transform_matrix != m_last_transform_matrix) {
        m_last_transform_matrix = transform_matrix;
        m_view_dirty = true;
    }

    if (m_view_dirty) {
        m_view_matrix = glm::affineInverse(transform_matrix);
    }

    if (m_proj_dirty) {
        const sge::Rect& projection_area = get_projection_area();
        m_projection_matrix = glm::orthoRH_ZO(
            projection_area.min.x, projection_area.max.x,
            projection_area.max.y, projection_area.min.y,
            0.0f, 1000.0f
        );
    }

    if (m_view_dirty || m_proj_dirty) {
        m_view_proj_matrix = m_projection_matrix * m_view_matrix;
        m_inv_view_proj_matrix = glm::inverse(m_view_proj_matrix);
    }

    m_view_dirty = false;
    m_proj_dirty = false;
}

glm::vec2 sge::Camera::screen_to_world(const glm::vec2& screen_pos) const {
    const glm::vec2 viewport = glm::vec2(m_viewport);

    const glm::vec2 inverted_y = glm::vec2(screen_pos.x, viewport.y - screen_pos.y);
    const glm::vec2 ndc = inverted_y * 2.0f / viewport - glm::vec2(1.0);
    const glm::mat4 ndc_to_world = m_inv_view_proj_matrix;
    const glm::vec2 world_pos = project_point(ndc_to_world, ndc);
    return world_pos;
}