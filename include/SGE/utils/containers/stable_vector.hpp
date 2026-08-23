#ifndef SGE_UTILS_CONTAINERS_STABLE_VECTOR_HPP_
#define SGE_UTILS_CONTAINERS_STABLE_VECTOR_HPP_

#include <vector>

namespace sge {

template <typename T>
class StableVector {
public:
    size_t push_back(const T& val) {
        size_t stable_id;

        if (!m_free_ids.empty()) {
            stable_id = m_free_ids.back();
            m_free_ids.pop_back();
            m_lookup[stable_id] = m_data.size();
        } else {
            stable_id = m_lookup.size();
            m_lookup.push_back(m_data.size());
        }

        m_data.push_back({val, stable_id});
        return stable_id;
    }

    void erase(size_t stable_id) {
        SGE_ASSERT(stable_id < m_lookup.size() && m_lookup[stable_id] != size_t(-1));

        size_t dead_data_idx = m_lookup[stable_id];
        size_t last_data_idx = m_data.size() - 1;

        if (dead_data_idx != last_data_idx) {
            m_data[dead_data_idx] = std::move(m_data[last_data_idx]);

            size_t moved_stable_id = m_data[dead_data_idx].lookup_idx;
            m_lookup[moved_stable_id] = dead_data_idx;
        }

        m_data.pop_back();
        m_lookup[stable_id] = size_t(-1);
        m_free_ids.push_back(stable_id);
    }

    T& operator[](size_t stable_id) {
        return m_data[m_lookup[stable_id]].value;
    }

    const T& operator[](size_t stable_id) const {
        return m_data[m_lookup[stable_id]].value;
    }

    [[nodiscard]]
    size_t size() const {
        return m_data.size();
    }

private:
    struct Slot {
        T value;
        size_t lookup_idx;
    };

    std::vector<Slot> m_data;
    std::vector<size_t> m_lookup;
    std::vector<size_t> m_free_ids;
};

} // namespace sge

#endif // SGE_UTILS_CONTAINERS_STABLE_VECTOR_HPP_