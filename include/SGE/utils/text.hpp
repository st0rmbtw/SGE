#ifndef _SGE_UTILS_TEXT_HPP_
#define _SGE_UTILS_TEXT_HPP_

#include "SGE/defines.hpp"
#include <glm/vec2.hpp>

#include <SGE/types/font.hpp>
#include <SGE/types/rich_text.hpp>

_SGE_BEGIN

glm::vec2 calculate_text_bounds(const Font& font, size_t length, const char* text, float size);

inline glm::vec2 calculate_text_bounds(const Font& font, float size, std::string_view string) {
    return calculate_text_bounds(font, string.size(), string.data(), size);
}

inline glm::vec2 calculate_text_bounds(const Font& font, const RichTextSection* sections, const size_t size) {
    glm::vec2 bounds = glm::vec2(0.0f);

    for (size_t i = 0; i < size; ++i) {
        const RichTextSection& section = sections[i];
        if (section.text.ends_with('\n')) {
            bounds.y += section.size;
        }
        bounds = glm::max(bounds, calculate_text_bounds(font, section.text.size(), section.text.data(), section.size));
    }

    return bounds;
}

template <size_t _Size>
inline glm::vec2 calculate_text_bounds(const Font& font, const RichText<_Size> text) {
    return calculate_text_bounds(font, text.data(), text.size());
}

_SGE_END

#endif