#ifndef SGE_UTILS_STRING_HPP_
#define SGE_UTILS_STRING_HPP_

#include <format>
#include <string>

namespace sge {

template <class... TArgs>
const std::string& TempFormat(const std::format_string<TArgs...> fmt, TArgs&&... args) {
    static std::string buffer = {};
    buffer.clear();
    std::format_to(std::back_inserter(buffer), fmt, std::forward<TArgs>(args)...);
    return buffer;
}

} // namespace sge

#endif // SGE_UTILS_STRING_HPP_
