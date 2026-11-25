#ifndef SGE_UTILS_HASH_HPP_
#define SGE_UTILS_HASH_HPP_

#include <cstddef>
#include <functional>
#include <ranges>
#include <span>

namespace sge {

inline constexpr uint64_t HASH_INIT = 1469598103934665603ULL;

inline void hash_fnv1a(std::size_t& hash, const void* data, size_t dataSize) {
    auto dataBytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < dataSize; ++i) {
        hash ^= dataBytes[i];
        hash *= 1099511628211ULL;
    }
}

template <std::ranges::contiguous_range R>
    requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
inline void hash_fnv1a(std::size_t& hash, const R& range) {
    auto span = std::as_bytes(std::span{range});
    return hash_fnv1a(hash, span.data(), span.size_bytes());
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace sge

#endif