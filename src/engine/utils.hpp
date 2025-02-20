#ifndef _ENGINE_UTILS_HPP_
#define _ENGINE_UTILS_HPP_

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <string>

#include "types/font.hpp"

#include "defines.hpp"
#include "log.hpp"

template <typename T> 
T* checked_alloc(size_t count) {
    T* _ptr = (T*) malloc(count * sizeof(T));
    if (_ptr == nullptr && count > 0) {
        LOG_ERROR("Out of memory");
        abort();
    }
    return _ptr;
}

static bool FileExists(const char *path) {
#ifdef PLATFORM_WINDOWS
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

uint32_t next_utf8_codepoint(const char* text, size_t& index);

glm::vec2 calculate_text_bounds(const Font& font, const std::string &text, float size);

#endif