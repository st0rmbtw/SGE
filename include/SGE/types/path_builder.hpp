#ifndef SGE_TYPES_PATH_BUILDER_HPP_
#define SGE_TYPES_PATH_BUILDER_HPP_

#include <SGE/assert.hpp>

#include "path.hpp"

namespace sge {

class PathBuilder {
public:
    PathBuilder& MoveTo(glm::vec2 position);
    PathBuilder& LineTo(glm::vec2 to);
    PathBuilder& QuadraticBezierTo(glm::vec2 ctrl, glm::vec2 to);
    PathBuilder& CubicBezierTo(glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to);

    PathBuilder& VerticalTo(float y) {
        LineTo(glm::vec2(GetLastPoint().x, y));
        return *this;
    }
    PathBuilder& HorizontalTo(float x) {
        LineTo(glm::vec2(x, GetLastPoint().y));
        return *this;
    }

    PathBuilder& MoveToRel(glm::vec2 d) {
        SGE_ASSERT(!m_points.empty());
        return MoveTo(GetLastPoint() + d);
    }
    PathBuilder& LineToRel(glm::vec2 d) {
        SGE_ASSERT(!m_points.empty());
        return LineTo(GetLastPoint() + d);
    }
    PathBuilder& QuadraticBezierToRel(glm::vec2 d1, glm::vec2 d2) {
        SGE_ASSERT(!m_points.empty());
        glm::vec2 p = GetLastPoint();
        return QuadraticBezierTo(p + d1, p + d2);
    }
    PathBuilder& CubicBezierToRel(glm::vec2 d1, glm::vec2 d2, glm::vec2 d3) {
        SGE_ASSERT(!m_points.empty());
        glm::vec2 p = GetLastPoint();
        return CubicBezierTo(p + d1, p + d2, p + d3);
    }
    PathBuilder& VerticalToRel(float dy) {
        SGE_ASSERT(!m_points.empty());
        LineToRel(glm::vec2(0.0f, dy));
        return *this;
    }
    PathBuilder& HorizontalToRel(float dx) {
        SGE_ASSERT(!m_points.empty());
        LineToRel(glm::vec2(dx, 0.0f));
        return *this;
    }

    PathBuilder& Close();

    sge::Path Build();

    void Reset() {
        m_commands.clear();
        m_points.clear();
        m_first = glm::vec2(0.f);
        m_open = false;
    }

private:
    glm::vec2 GetLastPoint() const {
        return m_points.back();
    }

    void PushPoint(glm::vec2 p) {
        m_points.push_back(p);
    }

private:
    std::vector<Path::Command> m_commands;
    std::vector<glm::vec2> m_points;
    glm::vec2 m_first{ 0.f };
    bool m_open{ false };
};

};

#endif // SGE_TYPES_PATH_BUILDER_HPP_
