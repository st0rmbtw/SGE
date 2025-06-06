#ifndef _SGE_UTILS_IO_HPP_
#define _SGE_UTILS_IO_HPP_

#include <cstdio>

#include <SGE/defines.hpp>
#include <SGE/log.hpp>

_SGE_BEGIN

inline bool FileExists(const char *path) {
#if SGE_PLATFORM_WINDOWS
    FILE *file = NULL;
    fopen_s(&file, path, "r");
#else
    FILE *file = fopen(path, "r");
#endif

    const bool exists = file != nullptr;

    if (exists) {
        fclose(file);
    }

    return exists;
}

_SGE_END

#endif