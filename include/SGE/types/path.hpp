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
        MoveTo = 0,
        LineTo,
        QuadraticBezierTo,
        CubicBezierTo,
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
    Path() = default;

    Path(std::vector<glm::vec2> points, std::vector<Command> commands) :
        m_commands{ std::move(commands) },
        m_points{ std::move(points) }
    {}

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

private:
    std::vector<Command> m_commands;
    std::vector<glm::vec2> m_points;
};

} // namespace sge

#endif
