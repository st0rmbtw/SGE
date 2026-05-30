#ifndef SGE_UTILS_STRING_HPP_
#define SGE_UTILS_STRING_HPP_

#include <string>
#include <format>

namespace sge {

template <class... _Types>
const std::string& TempFormat(const std::format_string<_Types...> fmt, _Types&&... args) {
    static std::string buffer = {};
    buffer.clear();
    std::format_to(std::back_inserter(buffer), fmt, std::forward<_Types>(args)...);
    return buffer;
}

}

#endif // SGE_UTILS_STRING_HPP_