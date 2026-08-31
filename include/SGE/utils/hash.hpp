#ifndef SGE_UTILS_HASH_HPP_
#define SGE_UTILS_HASH_HPP_

#include <cstddef>
#include <functional>

namespace sge {

inline constexpr uint64_t HASH_INIT = 1469598103934665603ULL;

template <typename T> requires std::is_trivially_copyable_v<T>
inline void hash_fnv1a(std::size_t& hash, const T* data, size_t count = 1) {
    const auto* bytes = reinterpret_cast<const uint8_t*>(data);
    const size_t size_bytes = sizeof(T) * count;
    for (size_t i = 0; i < size_bytes; ++i) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace sge

#endif