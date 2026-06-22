#ifndef SGE_RENDERER_PATH_HPP_
#define SGE_RENDERER_PATH_HPP_

#include <vector>

#include <glm/vec2.hpp>

#include <SGE/assert.hpp>
#include <SGE/math/rect.hpp>

namespace sge {

class Path {
public:
    enum class CommandType : uint8_t {
        Begin = 0,
        LineTo,
        QuadraticBezierTo,
        CubicBezierTo,
        End,
        Close
    };

    class Command { 
    public:
        Command(CommandType type, size_t index) {
            // SGE_ASSERT(m_index < (1LLU << 56LLU) - 1LLU);
            m_index = (static_cast<size_t>(type) << (sizeof(size_t)*8 - 8)) | (index & 0xffffffffffffff);
        }

        [[nodiscard]]
        inline size_t GetIndex() const {
            return m_index & 0xffffffffffffff;
        }
        
        [[nodiscard]]
        inline CommandType GetType() const {
            return static_cast<CommandType>((m_index >> 56) & 0xFF);
        }
    private:
        size_t m_index = 0;
    };

public:
    void Begin(glm::vec2 position);
    void LineTo(glm::vec2 to);
    void QuadraticBezierTo(glm::vec2 ctrl, glm::vec2 to);
    void CubicBezierTo(glm::vec2 ctrl1, glm::vec2 ctrl2, glm::vec2 to);
    void End();
    void Close();

    inline void VerticalTo(float y) {
        LineTo(glm::vec2(GetLastPoint().x, y));
    }

    inline void HorizontalTo(float x) {
        LineTo(glm::vec2(x, GetLastPoint().y));
    }

    inline void SetScale(glm::vec2 scale) noexcept {
        m_scale = scale;
        m_dirty = true;
    }

    [[nodiscard]]
    inline const std::vector<glm::vec2>& GetPoints() const {
        return m_points;
    }

    [[nodiscard]]
    inline const std::vector<Command>& GetCommands() const {
        return m_commands;
    }

    [[nodiscard]]
    inline glm::vec2 GetPoint(size_t i) const {
        return m_points[i];
    }

    [[nodiscard]]
    inline sge::Rect GetBounds() const noexcept {
        return m_bounds * m_scale;
    }

    [[nodiscard]]
    inline glm::vec2 GetScale() const noexcept {
        return m_scale;
    }

    [[nodiscard]]
    inline const std::vector<glm::vec2>& GetTriangleVertices(float tolerance = 0.1f) const noexcept {
        if (m_dirty) {
            Triangulate(tolerance);
        }

        return m_vertices;
    }

private:
    [[nodiscard]]
    inline glm::vec2 GetLastPoint() const {
        return m_points.back();
    }

    inline void AddPoint(glm::vec2 point) {
        m_points.push_back(point);
        m_bounds.min = glm::max(point, m_bounds.min);
        m_bounds.max = glm::min(point, m_bounds.max);
        m_dirty = true;
    }

    void Triangulate(float tolerance) const;

private:
    std::vector<Command> m_commands;
    std::vector<glm::vec2> m_points;
    mutable std::vector<glm::vec2> m_vertices;
    sge::Rect m_bounds;
    glm::vec2 m_first = glm::vec2(0.f);
    glm::vec2 m_scale = glm::vec2(1.f);
    mutable bool m_dirty = true;
};

} // namespace sge

#endif