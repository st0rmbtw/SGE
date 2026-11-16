#ifndef SGE_UTILS_CONTAINERS_DENSE_STORAGE_HPP_
#define SGE_UTILS_CONTAINERS_DENSE_STORAGE_HPP_

#include <span>
#include <vector>

#include <SGE/utils/containers/sparse_set.hpp>

namespace sge {

template <typename T>
class Handle {
public:
    Handle() = default;

    explicit Handle(SparseSetID id) : m_id{id} {}

    [[nodiscard]]
    bool IsValid() const {
        return m_id.IsValid();
    }

    [[nodiscard]]
    SparseSetID GetID() const {
        return m_id;
    }

    friend bool operator==(const Handle& a, const Handle& b) {
        return a.m_id == b.m_id;
    }

private:
    SparseSetID m_id;
};

template <typename THandle, typename... Ts>
class DenseStorage {
public:
    THandle add(Ts... values) {
        SparseSetID id = m_sparse_set.create();
        add_impl(std::make_tuple(std::move(values)...), std::index_sequence_for<Ts...>{});
        return THandle(id);
    }

    bool contains(THandle handle) const {
        return m_sparse_set.contains(handle.GetID());
    }

    void remove(THandle handle) {
        uint32_t dead = m_sparse_set.index_of(handle.GetID());
        uint32_t last = m_sparse_set.size() - 1;
        if (dead != last) {
            std::apply([](auto&&... args) { ((args[dead] = std::move(args[last])), ...); }, m_vectors);
        }
        std::apply([](auto&&... args) { (args.pop_back(), ...); }, m_vectors);
        m_sparse_set.remove(handle.GetID());
    }

    void clear() {
        std::apply([](auto&&... args) { (args.clear(), ...); }, m_vectors);
        m_sparse_set.clear();
    }

    template <typename T>
    T& get(THandle handle) {
        SGE_ASSERT(contains(handle));
        const auto& vec = std::get<T>(m_vectors);
        return vec[m_sparse_set.index_of(handle.GetID())];
    }

    template <typename T>
    const T& get(THandle handle) const {
        SGE_ASSERT(contains(handle));
        const auto& vec = std::get<T>(m_vectors);
        return vec[m_sparse_set.index_of(handle.GetID())];
    }

    template <typename T>
    std::span<const T> iterator() const {
        const auto& vec = std::get<T>(m_vectors);
        return std::span(vec.cdata(), m_sparse_set.size());
    }

    template <typename T>
    std::span<T> iterator() {
        auto& vec = std::get<T>(m_vectors);
        return std::span(vec.data(), m_sparse_set.size());
    }

    [[nodiscard]]
    size_t size() const {
        return m_sparse_set.size();
    }

private:
    template <std::size_t... Is>
    void add_impl(std::tuple<Ts...> value, std::index_sequence<Is...>) {
        (std::get<Is>(m_vectors).push_back(std::move(std::get<Is>(value))), ...);
    }

private:
    std::tuple<std::vector<Ts>...> m_vectors;
    SparseSet m_sparse_set;
};

template <typename THandle, typename T>
class DenseStorage<THandle, T> {
public:
    THandle add(T value) {
        SparseSetID id = m_sparse_set.create();
        m_data.push_back(std::move(value));
        return THandle(id);
    }

    bool contains(THandle handle) const {
        return m_sparse_set.contains(handle.GetID());
    }

    void remove(THandle handle) {
        uint32_t dead = m_sparse_set.index_of(handle.GetID());
        uint32_t last = m_sparse_set.size() - 1;
        if (dead != last) {
            m_data[dead] = std::move(m_data[last]);
        }
        m_data.pop_back();
        m_sparse_set.remove(handle.GetID());
    }

    void clear() {
        m_data.clear();
        m_sparse_set.clear();
    }

    T& get(THandle handle) {
        SGE_ASSERT(contains(handle));
        return m_data[m_sparse_set.index_of(handle.GetID())];
    }

    const T& get(THandle handle) const {
        SGE_ASSERT(contains(handle));
        return m_data[m_sparse_set.index_of(handle.GetID())];
    }

    std::vector<T>::const_iterator cbegin() const {
        return m_data.cbegin();
    }

    std::vector<T>::const_iterator cend() const {
        return std::next(m_data.cbegin(), m_sparse_set.size());
    }

    std::vector<T>::iterator begin() {
        return m_data.begin();
    }

    std::vector<T>::iterator end() {
        return std::next(m_data.begin(), m_sparse_set.size());
    }

    [[nodiscard]]
    size_t size() const {
        return m_sparse_set.size();
    }

private:
    std::vector<T> m_data;
    SparseSet m_sparse_set;
};

} // namespace sge

template<typename Tag>
struct std::hash<sge::Handle<Tag>> {
    size_t operator()(const sge::Handle<Tag>& h) const noexcept {
        return std::hash<sge::SparseSetID>{}(h.GetID());
    }
};

#endif // SGE_UTILS_CONTAINERS_STABLE_VECTOR_HPP_
