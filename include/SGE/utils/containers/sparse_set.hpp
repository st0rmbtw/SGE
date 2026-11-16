#ifndef SGE_UTILS_CONTAINERS_SPARSE_SET_HPP_
#define SGE_UTILS_CONTAINERS_SPARSE_SET_HPP_

#include <cstdint>
#include <vector>

#include <SGE/assert.hpp>

namespace sge {

class SparseSetID {
    static constexpr uint64_t INVALID = static_cast<uint64_t>(-1);
    static constexpr uint32_t INDEX_BITS = 20;
    static constexpr uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1;

public:
    SparseSetID() = default;

    explicit SparseSetID(uint32_t index, uint32_t generation) :
        m_value{ (static_cast<uint64_t>(generation) << INDEX_BITS) | (index & INDEX_MASK) }
    {}

    [[nodiscard]]
    uint32_t Index() const {
        return (m_value & INDEX_MASK);
    }

    [[nodiscard]]
    uint32_t Generation() const {
        return (m_value >> INDEX_BITS);
    }

    [[nodiscard]]
    bool IsValid() const {
        return m_value != INVALID;
    }

    [[nodiscard]]
    uint64_t GetValue() const {
        return m_value;
    }

    friend bool operator==(const SparseSetID& a, const SparseSetID& b) {
        return a.m_value == b.m_value;
    }

private:
    uint64_t m_value = INVALID;
};

class SparseSet {
public:
    SparseSetID create() {
        if (m_size < m_dense.size()) {
            SparseSetID recycled = m_dense[m_size];
            m_sparse[recycled.Index()] = m_size;
            m_size += 1;
            return recycled;
        }

        uint32_t index = m_dense.size();
        auto id = SparseSetID(index, 0);
        m_sparse.push_back(m_size);
        m_dense.push_back(id);
        m_size += 1;
        return id;
    }

    [[nodiscard]]
    bool contains(SparseSetID id) const {
        if (!id.IsValid()) return false;
        uint32_t idx = id.Index();
        if (idx >= m_sparse.size()) return false;
        uint32_t dense_idx = m_sparse[idx];
        return dense_idx < m_size && m_dense[dense_idx] == id;
    }

    void remove(SparseSetID id) {
        SGE_ASSERT(contains(id));

        uint32_t dead_dense_idx = m_sparse[id.Index()];
        uint32_t last_alive_idx = m_size - 1;

        if (dead_dense_idx != last_alive_idx) {
            // Swap the destroyed handle with the last alive handle
            SparseSetID last_alive = m_dense[last_alive_idx];
            m_dense[dead_dense_idx] = last_alive;
            m_sparse[last_alive.Index()] = dead_dense_idx;
        }

        // Increment generation and place in the dead partition
        auto bumped = SparseSetID(id.Index(), id.Generation() + 1);
        m_dense[last_alive_idx] = bumped;
        m_sparse[id.Index()] = last_alive_idx;

        m_size -= 1;
    }

    [[nodiscard]]
    uint32_t index_of(SparseSetID id) const {
        SGE_ASSERT(contains(id));
        return m_sparse[id.Index()];
    }

    void clear() {
        m_dense.clear();
        m_sparse.clear();
        m_size = 0;
    }

    [[nodiscard]]
    size_t size() const {
        return m_size;
    }

    [[nodiscard]]
    std::vector<SparseSetID>::const_iterator begin() const {
        return m_dense.cbegin();
    }

    [[nodiscard]]
    std::vector<SparseSetID>::const_iterator end() const {
        return std::next(m_dense.cbegin(), m_size);
    }

private:
    std::vector<SparseSetID> m_dense;
    std::vector<uint32_t> m_sparse;
    size_t m_size = 0;
};

} // namespace sge

template<>
struct std::hash<sge::SparseSetID> {
    size_t operator()(const sge::SparseSetID& id) const noexcept {
        return std::hash<uint64_t>{}(id.GetValue());
    }
};

#endif // SGE_UTILS_CONTAINERS_SPARSE_SET_HPP_
