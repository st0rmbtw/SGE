#ifndef _SGE_UTILS_TEXT_HPP_
#define _SGE_UTILS_TEXT_HPP_

#include "SGE/defines.hpp"
#include <glm/vec2.hpp>

#include <SGE/types/font.hpp>
#include <SGE/types/rich_text.hpp>

_SGE_BEGIN

glm::vec2 calculate_text_bounds(const Font& font, size_t length, const char* text, float size);

template <size_t SIZE>
inline glm::vec2 calculate_text_bounds(const Font& font, const RichText<SIZE> text) {
    glm::vec2 bounds = glm::vec2(0.0f);

    for (const RichTextSection& section : text.sections()) {
        bounds = glm::max(bounds, calculate_text_bounds(font, section.text.size(), section.text.data(), section.size));
    }

    return bounds;
}

_SGE_END

#endif