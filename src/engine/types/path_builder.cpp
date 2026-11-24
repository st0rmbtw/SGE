#include <SGE/types/path_builder.hpp>
#include <SGE/profile.hpp>

#include <glm/gtx/norm.hpp>

using CT = sge::Path::CommandType;

sge::PathBuilder& sge::PathBuilder::MoveTo(glm::vec2 position) {
    if (m_open)
        Close();

    m_open = true;
    m_first = position;

    if (!m_commands.empty()) {
        auto& c = m_commands.back();
        if (c.GetType() == CT::MoveTo) {
            m_points[c.GetIndex()] = position;
            return *this;
        }
    }

    m_commands.emplace_back(CT::MoveTo, m_points.size());
    PushPoint(position);

    return *this;
}

sge::PathBuilder& sge::PathBuilder::LineTo(glm::vec2 to) {
    SGE_ASSERT_M(m_open, "Subpath is not open! Forgot the MoveTo command?");
    m_commands.emplace_back(CT::LineTo, m_points.size());
    PushPoint(to);
    return *this;
}

sge::PathBuilder& sge::PathBuilder::QuadraticBezierTo(glm::vec2 ctrl, glm::vec2 to) {
    SGE_ASSERT_M(m_open, "Subpath is not open! Forgot the MoveTo command?");
    m_commands.emplace_back(CT::QuadraticBezierTo, m_points.size());
    PushPoint(ctrl);
    PushPoint(to);
    return *this;
}

sge::PathBuilder& sge::PathBuilder::CubicBezierTo(glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to) {
    SGE_ASSERT_M(m_open, "Subpath is not open! Forgot the MoveTo command?");
    m_commands.emplace_back(CT::CubicBezierTo, m_points.size());
    PushPoint(ctrl1);
    PushPoint(ctrl2);
    PushPoint(to);
    return *this;
}

sge::PathBuilder& sge::PathBuilder::Close() {
    if (!m_open)
        return *this;

    m_open = false;
    m_commands.emplace_back(CT::Close, m_points.size());
    PushPoint(m_first);
    return *this;
}

sge::Path sge::PathBuilder::Build() {
    Close();
    return {m_points, m_commands};
}
