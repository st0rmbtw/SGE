#ifndef _SGE_UTILS_HPP_
#define _SGE_UTILS_HPP_

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <SGE/types/rich_text.hpp>
#include <SGE/types/font.hpp>

#include "defines.hpp"
#include "log.hpp"

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

uint32_t next_utf8_codepoint(const char* text, size_t& index);

glm::vec2 calculate_text_bounds(const Font& font, size_t length, const char* text, float size);

template <size_t Size>
inline glm::vec2 calculate_text_bounds(const Font& font, const RichText<Size> text) {
    glm::vec2 bounds = glm::vec2(0.0f);

    for (const RichTextSection& section : text.sections()) {
        bounds = glm::max(bounds, calculate_text_bounds(font, section.text.size(), section.text.data(), section.size));
    }

    return bounds;
}

_SGE_END

#endif