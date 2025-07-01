#include <SGE/renderer/camera.hpp>
#include <SGE/engine.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace sge;

void Camera::update_projection_area() {
    glm::vec2 viewport = glm::vec2(m_viewport);

    switch (m_origin) {
    case CameraOrigin::TopLeft: {
        glm::vec2 min;
        min.x = m_flip_horizontal ? viewport.x : 0.0f;
        min.y = m_flip_vertical ? viewport.y : 0.0f;

        glm::vec2 max;
        max.x = m_flip_horizontal ? 0.0f : viewport.x;
        max.y = m_flip_vertical ? 0.0f : viewport.y;

        m_area = sge::Rect(
            min * m_zoom,
            max * m_zoom
        );

        m_area_nozoom = sge::Rect(min, max);
    } break;

    case CameraOrigin::Center: {
        glm::vec2 min;
        min.x = (m_flip_horizontal ? viewport.x : -viewport.x) / 2.0f;
        min.y = (m_flip_vertical ? viewport.y : -viewport.y) / 2.0f;

        glm::vec2 max;
        max.x = (m_flip_horizontal ? -viewport.x : viewport.x) / 2.0f;
        max.y = (m_flip_vertical ? -viewport.y : viewport.y) / 2.0f;

        m_area = sge::Rect::from_corners(
            min * m_zoom,
            max * m_zoom
        );
        m_area_nozoom = sge::Rect::from_corners(min, max);
    } break;
    }
}

void Camera::compute_projection_and_view_matrix() {
    const sge::Rect& projection_area = get_projection_area();
    const sge::Rect& nozoom_projection_area = get_nozoom_projection_area();

    if (sge::Engine::Renderer().Backend().IsOpenGL()) {
        const glm::vec3 eye     = glm::vec3(m_position, 50.0f);
        const glm::vec3 right   = glm::vec3(m_right, 0.0f, 0.0f);
        const glm::vec3 up      = glm::vec3(0.0f, m_up, 0.0f);
        const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -m_forward);

        m_view_matrix = glm::inverse(glm::mat4(
            glm::vec4(right, 0.0f),
            glm::vec4(up, 0.0f),
            glm::vec4(forward, 0.0f),
            glm::vec4(eye, 1.0f)
        ));

        m_projection_matrix = glm::orthoRH_NO(
            projection_area.min.x, projection_area.max.x,
            projection_area.min.y, projection_area.max.y,
            0.0f, 100.0f
        );
        m_nozoom_projection_matrix = glm::orthoRH_NO(
            nozoom_projection_area.min.x, nozoom_projection_area.max.x,
            nozoom_projection_area.min.y, nozoom_projection_area.max.y,
            0.0f, 100.0f
        );
    } else {
        const glm::vec3 eye     = glm::vec3(m_position, 50.0f);
        const glm::vec3 right   = glm::vec3(m_right, 0.0f, 0.0f);
        const glm::vec3 up      = glm::vec3(0.0f, m_up, 0.0f);
        const glm::vec3 forward = glm::vec3(0.0f, 0.0f, m_forward);

        m_view_matrix = glm::inverse(glm::mat4(
            glm::vec4(right, 0.0f),
            glm::vec4(up, 0.0f),
            glm::vec4(forward, 0.0f),
            glm::vec4(eye, 1.0f)
        ));

        m_projection_matrix = glm::orthoLH_ZO(
            projection_area.min.x, projection_area.max.x,
            projection_area.min.y, projection_area.max.y,
            0.0f, 100.0f
        );
        m_nozoom_projection_matrix = glm::orthoLH_ZO(
            nozoom_projection_area.min.x, nozoom_projection_area.max.x,
            nozoom_projection_area.min.y, nozoom_projection_area.max.y,
            0.0f, 100.0f
        );
    }

    m_view_proj_matrix = m_projection_matrix * m_view_matrix;
    m_nozoom_view_proj_matrix = m_nozoom_projection_matrix * m_view_matrix;
    m_inv_view_proj_matrix = glm::inverse(m_projection_matrix * m_view_matrix);
}

static inline glm::vec2 project_point(const glm::mat4& mat, const glm::vec2& point) {
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