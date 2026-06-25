#ifndef _SGE_UTILS_TEXT_HPP_
#define _SGE_UTILS_TEXT_HPP_

#include <glm/common.hpp>
#include <glm/vec2.hpp>

#include <SGE/types/font.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/utils/utf8.hpp>

namespace sge {

struct FitResult {
    size_t bytes;
    uint32_t char_count;
};

float MeasureTextHeight(const Font& font, float size, const char* text, size_t length) noexcept;

inline float MeasureTextHeight(const Font& font, float size, std::string_view string) noexcept {
    return MeasureTextHeight(font, size, string.data(), string.size());
}

glm::vec2 MeasureText(const Font& font, float size, const char* text, size_t length);
inline glm::vec2 MeasureText(const Font& font, float size, std::string_view string) {
    return MeasureText(font, size, string.data(), string.size());
}

FitResult CharsFitInLineFromStart(const Font& font, float size, const char* text, size_t length, float line_width) noexcept;
inline FitResult CharsFitInLineFromStart(const Font& font, float size, std::string_view string, float line_width) noexcept {
    return CharsFitInLineFromStart(font, size, string.data(), string.size(), line_width);
}

FitResult CharsFitInLineFromEnd(const Font& font, float size, const char* text, size_t length, float line_width) noexcept;
inline FitResult CharsFitInLineFromEnd(const Font& font, float size, std::string_view string, float line_width) noexcept {
    return CharsFitInLineFromEnd(font, size, string.data(), string.size(), line_width);
}

inline glm::vec2 MeasureText(const Font& font, const RichTextSection* sections, const size_t size) {
    glm::vec2 bounds = glm::vec2(0.0f);

    for (size_t i = 0; i < size; ++i) {
        const RichTextSection& section = sections[i];
        if (section.text.ends_with('\n')) {
            bounds.y += section.size;
        }
        bounds = glm::max(bounds, MeasureText(font, section.size, section.text.data(), section.text.size()));
    }

    return bounds;
}

template <size_t Size>
inline glm::vec2 MeasureText(const Font& font, const RichText<Size> text) {
    return MeasureText(font, text.data(), text.size());
}

} // namespace sge

#endif