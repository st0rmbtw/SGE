#ifndef SGE_HANDLE_HPP_
#define SGE_HANDLE_HPP_

#include <cstdint>
#include <functional>

namespace sge {

template <typename Tag>
class Handle {
    static constexpr uint64_t INVALID = 0;

public:
    Handle() noexcept = default;

    explicit Handle(uint64_t id) noexcept : m_id(id) {}

    [[nodiscard]]
    uint64_t Id() const noexcept { return m_id; }

    [[nodiscard]]
    bool IsValid() const noexcept { return m_id != INVALID; }

    friend bool operator==(const Handle& a, const Handle& b) { return a.m_id == b.m_id; }
    friend bool operator!=(const Handle& a, const Handle& b) { return a.m_id != b.m_id; }

private:
    uint64_t m_id = INVALID;
};

class IdGenerator {
public:
    static uint64_t Next() noexcept { return ++s_counter; }
private:
    static inline uint64_t s_counter = 0;
};

} // namespace sge

template <typename Tag>
struct std::hash<sge::Handle<Tag>> {
    size_t operator()(const sge::Handle<Tag>& h) const noexcept {
        return std::hash<uint64_t>{}(h.Id());
    }
};

#endif // SGE_RENDERER_HPP_