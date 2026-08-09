#ifndef SGE_UTILS_HASH_HPP_
#define SGE_UTILS_HASH_HPP_

#include <cstddef>
#include <functional>

namespace sge {

inline uint64_t hash_fnv1a(const void* data, size_t size, uint64_t seed = 1469598103934665603ULL) {
    auto* bytes = static_cast<const uint8_t*>(data);
    uint64_t hash = seed;
    for (size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace sge

#endif