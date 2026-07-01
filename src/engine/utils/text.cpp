#include <SGE/profile.hpp>
#include <SGE/types/font.hpp>
#include <SGE/utils/text.hpp>
#include <SGE/utils/utf8.hpp>

namespace {

glm::vec2 MeasureTextInternal(const std::unordered_map<uint32_t, sge::Glyph>& glyphs, float line_height, float scale, const char* text, size_t length) {
    auto bounds = glm::vec2(0.0f);
    float x = 0.0f;

    bounds.y = line_height;

    uint32_t codepoint = 0;
    for (size_t i = 0; i < length;) {
        i += sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);

        if (codepoint == '\n') {
            bounds.y += line_height * scale;
            x = 0.0f;
            continue;
        }

        auto it = glyphs.find(codepoint);
        if (it == glyphs.end()) {
            it = glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += glyph.advance * scale;
        bounds.x = std::max(bounds.x, x);
        // bounds.y = std::max(bounds.y, (glyph.size.y + (glyph.size.y - glyph.bearing.y)) * scale);
    }

    return bounds;
}

float MeasureTextHeightInternal(float line_height, const char* text, size_t length) noexcept {
    float height = line_height;

    uint32_t codepoint = 0;
    for (size_t i = 0; i < length;) {
        i += sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);
        if (codepoint == '\n') {
            height += line_height;
        }
    }

    return height;
}

sge::FitResult CharsFitInLineFromStartInternal(
    const std::unordered_map<uint32_t, sge::Glyph>& glyphs,
    float scale,
    const char* text,
    size_t length,
    float line_width
) noexcept {
    uint32_t count = 0;
    uint32_t codepoint = 0;
    size_t total_bytes = 0;
    float width = 0.0f;
    float x = 0.0f;

    size_t i = 0;
    while (i < length) {
        const uint8_t bytes = sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);
        i += bytes;

        if (codepoint == '\n') {
            x = 0.0f;
            continue;
        }

        auto it = glyphs.find(codepoint);
        if (it == glyphs.end()) {
            it = glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += glyph.advance * scale;
        width = std::max(width, x);

        if (width >= line_width) {
            break;
        }

        count++;
        total_bytes += bytes;
    }

    return sge::FitResult{ .bytes = total_bytes, .char_count = count };
}

sge::FitResult CharsFitInLineFromEndInternal(
    const std::unordered_map<uint32_t, sge::Glyph>& glyphs,
    float scale,
    const char* text,
    size_t length,
    float line_width
) noexcept {
    ZoneScoped;

    uint32_t count = 0;
    uint32_t codepoint = 0;
    size_t total_bytes = 0;
    float width = 0.0f;
    float x = 0.0f;

    int i = length;
    while (i > 0) {
        const uint8_t bytes = sge::count_utf8_char_bytes_from_end(text, i);
        i -= bytes;
        sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);

        if (codepoint == '\n') {
            x = 0.0f;
            continue;
        }

        auto it = glyphs.find(codepoint);
        if (it == glyphs.end()) {
            it = glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += glyph.advance * scale;
        width = std::max(width, x);

        if (width >= line_width) {
            break;
        }

        count++;
        total_bytes += bytes;
    }

    return sge::FitResult{ .bytes = total_bytes, .char_count = count };
}

} // namespace

glm::vec2 sge::MeasureText(const sge::FontVector& font, float size, const char* text, size_t length) {
    ZoneScoped;
    const float scale = size / font.units_per_em;
    const float line_height = (font.ascender - font.descender);
    return MeasureTextInternal(font.glyphs, line_height, scale, text, length);
}

glm::vec2 sge::MeasureText(const sge::Font& font, float size, const char* text, size_t length) {
    ZoneScoped;
    const float scale = size / font.font_size;
    const float line_height = (font.max_ascent + font.max_descent) * scale;
    return MeasureTextInternal(font.glyphs, line_height, scale, text, length);
}

float sge::MeasureTextHeight(const FontVector& font, float size, const char* text, size_t length) noexcept {
    ZoneScoped;
    const float scale = size / font.units_per_em;
    const float line_height = (font.ascender - font.descender) * scale;
    return MeasureTextHeightInternal(line_height, text, length);
}

float sge::MeasureTextHeight(const Font& font, float size, const char* text, size_t length) noexcept {
    ZoneScoped;
    const float scale = size / font.font_size;
    const float line_height = (font.max_ascent + font.max_descent) * scale;
    return MeasureTextHeightInternal(line_height, text, length);
}

sge::FitResult sge::CharsFitInLineFromStart(const sge::FontVector& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;
    const float scale = size / font.units_per_em;
    return CharsFitInLineFromStartInternal(font.glyphs, scale, text, length, line_width);
}

sge::FitResult sge::CharsFitInLineFromStart(const sge::Font& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;
    const float scale = size / font.font_size;
    return CharsFitInLineFromStartInternal(font.glyphs, scale, text, length, line_width);
}

sge::FitResult sge::CharsFitInLineFromEnd(const FontVector& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;
    const float scale = size / font.units_per_em;
    return CharsFitInLineFromEndInternal(font.glyphs, scale, text, length, line_width);
}

sge::FitResult sge::CharsFitInLineFromEnd(const Font& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;
    const float scale = size / font.font_size;
    return CharsFitInLineFromEndInternal(font.glyphs, scale, text, length, line_width);
}