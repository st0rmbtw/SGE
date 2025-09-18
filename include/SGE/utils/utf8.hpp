#ifndef _SGE_UTILS_UTF8_HPP_
#define _SGE_UTILS_UTF8_HPP_

#include <cstddef>

namespace sge {

uint32_t next_utf8_codepoint(const char* text, std::size_t& index);

}

#endif
