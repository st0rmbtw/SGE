#ifndef SGE_MACROS_HPP_
#define SGE_MACROS_HPP_

namespace sge::internal {
    template <typename... Args>
    inline constexpr void SuppressUnused(Args&&...) noexcept {}
}

#define SGE_UNUSED(...) sge::internal::SuppressUnused(__VA_ARGS__)

#endif