#ifndef _SGE_UTILS_ALLOC_HPP_
#define _SGE_UTILS_ALLOC_HPP_

#include <cstdio>
#include <cstdlib>

#include <SGE/defines.hpp>
#include <SGE/log.hpp>

_SGE_BEGIN

template <typename T>
T* checked_alloc(size_t count) {
    T* _ptr = (T*) malloc(count * sizeof(T));
    if (_ptr == nullptr && count > 0) {
        SGE_LOG_ERROR("Out of memory");
        abort();
    }
    return _ptr;
}

_SGE_END

#endif