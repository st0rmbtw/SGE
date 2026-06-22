#include <SGE/profile.hpp>
#include <SGE/types/path.hpp>

#include <cmath>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace {

struct BezierMap {
    float x0;
    float x2;
    float scale;
    float cross;
};

struct CubicBezierPolynomial {
    glm::vec2 a0;
    glm::vec2 a1;
    glm::vec2 a2;
    glm::vec2 a3;

    [[nodiscard]]
    glm::vec2 sample(float t) const {
        float t2 = t;
        glm::vec2 v = a0;
        v += a1 * t2;
        t2 *= t;
        v += a2 * t2;
        t2 *= t;
        v += a3 * t2;
        return v;
    }
};

glm::vec2 quadratic_bezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float t) {
    return (1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2;
}

float approx_integral(float x) {
    static constexpr float d = 0.67f;
    return x / (1 - d + std::pow(std::pow(d, 4) + 0.25f * x * x, 0.25f));
}

float approx_inv_integral(float x) {
    static constexpr float b = 0.39f;
    return x * (1 - b + std::sqrt(b * b + 0.25f * x * x));
}

BezierMap map_quadratic_bezier_to_basic(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2) {
    const float ddx = 2.0f * p1.x - p0.x - p2.x;
    const float ddy = 2.0f * p1.y - p0.y - p2.y;
    const float u0 = (p1.x - p0.x) * ddx + (p1.y - p0.y) * ddy;
    const float u2 = (p2.x - p1.x) * ddx + (p2.y - p1.y) * ddy;
    const float cross = (p2.x - p0.x) * ddy - (p2.y - p0.y) * ddx;
    const float x0 = u0 / cross;
    const float x2 = u2 / cross;
    const float scale = std::abs(cross) / (std::hypot(ddx, ddy) * std::abs(x2 - x0));
    return BezierMap {
        .x0 = x0,
        .x2 = x2,
        .scale = scale,
        .cross = cross,
    };
}

int calculate_cubic_bezier_segments_count(glm::vec2 from, glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to, float tolerance) {
    ZoneScoped;

    glm::vec2 v1 = (from - ctrl1 * 2.0f + ctrl2) * 6.0f;
    glm::vec2 v2 = (ctrl1 - ctrl2 * 2.0f + to) * 6.0f;
    float l = glm::max(glm::dot(v1, v1), glm::dot(v2, v2));
    float d = 1.0f / (8.0f * tolerance);
    float err4 = l * d * d;

    // Avoid two square roots using a lookup table that contains
    // i^4 for  i in 1..25.
    static constexpr uint8_t N = 24;
    static constexpr float LUT[N] = {
        1.0, 16.0, 81.0, 256.0, 625.0, 1296.0, 2401.0, 4096.0, 6561.0,
        10000.0, 14641.0, 20736.0, 28561.0, 38416.0, 50625.0, 65536.0,
        83521.0, 104976.0, 130321.0, 160000.0, 194481.0, 234256.0,
        279841.0, 331776.0
    };

    // If the value we are looking for is within the LUT, take the fast path
    if (err4 <= 331776.0) {
        for (uint8_t i = 0; i < N; ++i) {
            if (err4 <= LUT[i]) {
                return i + 1;
            }
        }
    }

    // Otherwise fall back to computing via two square roots.
    return std::max(static_cast<int>(std::sqrt(std::sqrt(err4))), 1);
}

CubicBezierPolynomial cubic_bezier_to_polynomial_form(glm::vec2 from, glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to) {
    return CubicBezierPolynomial {
        .a0 = from,
        .a1 = (ctrl1 - from) * 3.0f,
        .a2 = from * 3.0f - ctrl1 * 6.0f + ctrl2 * 3.0f,
        .a3 = to - from + (ctrl1 - ctrl2) * 3.0f
    };
}

void flatten_quadratic_bezier(std::vector<glm::vec2>& output, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float tolerance) {
    ZoneScoped;

    const BezierMap map = map_quadratic_bezier_to_basic(p0, p1, p2);
    const float a0 = approx_integral(map.x0);
    const float a2 = approx_integral(map.x2);
    const float count = 0.5f * std::abs(a2 - a0) * std::sqrt(map.scale / tolerance);
    const int n = std::ceil(count);
    // Handle case where all the points are collinear and the end point is between the start point and the control point
    if (!std::isfinite(n) || n == 0 || n == 1) {
        // Find t values where the derivative is 0
        const float divx = p0.x + p2.x - 2.0f * p1.x;
        const float divy = p0.y + p2.y - 2.0f * p1.y;
        const float tx = (p0.x - p1.x) / divx;
        const float ty = (p0.y - p1.y) / divy;

        float t = 0.0;
        if (std::isfinite(tx) && tx > 0.0f && tx < 1.0f) {
            t = tx;
            output.push_back(quadratic_bezier(p0, p1, p2, tx));
        }
        if (std::isfinite(ty) && ty > 0.0f && ty < 1.0f) {
            if (ty > t) {
                output.push_back(quadratic_bezier(p0, p1, p2, t));
            }
        }
        return;
    }
    const float u0 = approx_inv_integral(a0);
    const float u2 = approx_inv_integral(a2);
    for (int i = 1; i < n; i++) {
        const float u = approx_inv_integral(a0 + ((a2 - a0) * i) / n);
        const float t = (u - u0) / (u2 - u0);
        // const float t = (u - map.x0) / (map.x2 - map.x0);
        output.push_back(quadratic_bezier(p0, p1, p2, t));
    }
}

void flatten_cubic_bezier(std::vector<glm::vec2>& output, glm::vec2 from, glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to, float tolerance) {
    ZoneScoped;

    const int n = calculate_cubic_bezier_segments_count(from, ctrl1, ctrl2, to, tolerance);
    const float step = 1.0f / n;
    const CubicBezierPolynomial polynomial = cubic_bezier_to_polynomial_form(from, ctrl1, ctrl2, to);

    float t0 = 0.0f;
    for (int i = 0; i < n - 1; ++i) {
        const float t1 = t0 + step;
        output.push_back(polynomial.sample(t1));
        t0 = t1;
    }
}

} // namespace

void sge::Path::Triangulate(float tolerance) const {
    m_vertices.clear();

    std::vector<std::vector<glm::vec2>> contours;

    for (Path::Command command : GetCommands()) {
        switch (command.GetType()) {
        case Path::CommandType::Begin: {
            glm::vec2 p = GetPoint(command.GetIndex()) * m_scale;
            contours.emplace_back();
            contours.back().push_back(p);
            break;
        }
        case Path::CommandType::LineTo: {
            glm::vec2 p = GetPoint(command.GetIndex()) * m_scale;
            contours.back().push_back(p);
            break;
        }
        case Path::CommandType::QuadraticBezierTo: {
            glm::vec2 p0 = GetPoint(command.GetIndex() - 1) * m_scale;
            glm::vec2 p1 = GetPoint(command.GetIndex() + 0) * m_scale;
            glm::vec2 p2 = GetPoint(command.GetIndex() + 1) * m_scale;
            flatten_quadratic_bezier(contours.back(), p0, p1, p2, tolerance);
            break;
        }
        case Path::CommandType::CubicBezierTo: {
            glm::vec2 p0 = GetPoint(command.GetIndex() - 1) * m_scale;
            glm::vec2 p1 = GetPoint(command.GetIndex() + 0) * m_scale;
            glm::vec2 p2 = GetPoint(command.GetIndex() + 1) * m_scale;
            glm::vec2 p3 = GetPoint(command.GetIndex() + 2) * m_scale;
            flatten_cubic_bezier(contours.back(), p0, p1, p2, p3, tolerance);
            break;
        }
        case Path::CommandType::Close: {
            glm::vec2 p = GetPoint(command.GetIndex()) * m_scale;
            contours.back().push_back(p);
            break;
        }
        default:
            break;
        }
    }
    
    for (const auto& contour : contours) {
        glm::vec2 origin = contour[0];
        for (size_t i = 1; i + 1 < contour.size(); i++) {
            m_vertices.emplace_back(origin);
            m_vertices.emplace_back(contour[i]);
            m_vertices.emplace_back(contour[i+1]);
        }
    }

    m_dirty = false;
}

void sge::Path::Begin(glm::vec2 position) {
    m_first = position;
    m_commands.emplace_back(CommandType::Begin, m_points.size());
    AddPoint(position);
}

void sge::Path::LineTo(glm::vec2 to) {
    m_commands.emplace_back(CommandType::LineTo, m_points.size());
    AddPoint(to);
}

void sge::Path::QuadraticBezierTo(glm::vec2 ctrl, glm::vec2 to) {
    m_commands.emplace_back(CommandType::QuadraticBezierTo, m_points.size());
    AddPoint(ctrl);
    AddPoint(to);
}

void sge::Path::CubicBezierTo(glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to) {
    m_commands.emplace_back(CommandType::CubicBezierTo, m_points.size());
    AddPoint(ctrl1);
    AddPoint(ctrl2);
    AddPoint(to);
}

void sge::Path::End() {
    m_commands.emplace_back(CommandType::End, -1);
}

void sge::Path::Close() {
    m_commands.emplace_back(CommandType::Close, m_points.size());
    m_points.push_back(m_first);
}