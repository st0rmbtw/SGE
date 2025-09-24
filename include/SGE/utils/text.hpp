#ifndef _SGE_UTILS_TEXT_HPP_
#define _SGE_UTILS_TEXT_HPP_

#include <glm/vec2.hpp>

#include <SGE/types/font.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/profile.hpp>
#include <SGE/utils/utf8.hpp>

namespace sge {

struct FitResult {
    size_t bytes;
    uint32_t char_count;
};

inline float calculate_text_height(const Font& font, float size, const char* text, size_t length) noexcept {
    ZoneScoped;
    
    const float scale = size / font.font_size;
    float height = (font.max_ascent + font.max_descent) * scale;

    uint32_t codepoint = 0;
    for (size_t i = 0; i < length;) {
        i += sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);
        if (codepoint == '\n') {
            height += size;
        }
    }

    return height;
}

inline float calculate_text_height(const Font& font, float size, std::string_view string) noexcept {
    return calculate_text_height(font, size, string.data(), string.size());
}

glm::vec2 calculate_text_bounds(const Font& font, float size, const char* text, size_t length);
inline glm::vec2 calculate_text_bounds(const Font& font, float size, std::string_view string) {
    return calculate_text_bounds(font, size, string.data(), string.size());
}

FitResult chars_fit_in_line_from_start(const Font& font, float size, const char* text, size_t length, float line_width) noexcept;
inline FitResult chars_fit_in_line_from_start(const Font& font, float size, std::string_view string, float line_width) noexcept {
    return chars_fit_in_line_from_start(font, size, string.data(), string.size(), line_width);
}

FitResult chars_fit_in_line_from_end(const Font& font, float size, const char* text, size_t length, float line_width) noexcept;
inline FitResult chars_fit_in_line_from_end(const Font& font, float size, std::string_view string, float line_width) noexcept {
    return chars_fit_in_line_from_end(font, size, string.data(), string.size(), line_width);
}

inline glm::vec2 calculate_text_bounds(const Font& font, const RichTextSection* sections, const size_t size) {
    glm::vec2 bounds = glm::vec2(0.0f);

    for (size_t i = 0; i < size; ++i) {
        const RichTextSection& section = sections[i];
        if (section.text.ends_with('\n')) {
            bounds.y += section.size;
        }
        bounds = glm::max(bounds, calculate_text_bounds(font, section.size, section.text.data(), section.text.size()));
    }

    return bounds;
}

template <size_t _Size>
inline glm::vec2 calculate_text_bounds(const Font& font, const RichText<_Size> text) {
    return calculate_text_bounds(font, text.data(), text.size());
}

}

#endif