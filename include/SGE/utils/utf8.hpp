#ifndef _SGE_UTILS_UTF8_HPP_
#define _SGE_UTILS_UTF8_HPP_

#include <cstdint>

#include <SGE/defines.hpp>

_SGE_BEGIN

uint32_t next_utf8_codepoint(const char* text, size_t& index);

_SGE_END

#endif