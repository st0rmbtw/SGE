#ifndef SGE_UTILS_DATA_HPP_
#define SGE_UTILS_DATA_HPP_

#include <span>

namespace sge {

template <typename R>
inline std::span<const std::byte> as_bytes(const R& range) {
    return std::as_bytes(std::span(range));
}

}

#endif // SGE_UTILS_DATA_HPP_