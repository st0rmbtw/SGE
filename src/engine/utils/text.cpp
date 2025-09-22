#include <SGE/utils/text.hpp>
#include <SGE/utils/utf8.hpp>
#include <SGE/profile.hpp>

glm::vec2 sge::calculate_text_bounds(const sge::Font& font, float size, const char* text, size_t length) {
    ZoneScoped;

    auto bounds = glm::vec2(0.0f);
    float x = 0.0f;

    const float scale = size / font.font_size;

    bounds.y = (font.max_ascent + font.max_descent) * scale;

    uint32_t codepoint = 0;
    for (size_t i = 0; i < length;) {
        i += sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);

        if (codepoint == '\n') {
            bounds.y += size;
            x = 0.0f;
            continue;
        }

        auto it = font.glyphs.find(codepoint);
        if (it == font.glyphs.end()) {
            it = font.glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += (glyph.advance >> 6) * scale;
        bounds.x = std::max(bounds.x, x);
        // bounds.y = std::max(bounds.y, (glyph.size.y + (glyph.size.y - glyph.bearing.y)) * scale);
    }

    return bounds;
}

uint32_t sge::chars_fit_in_line_from_start(const sge::Font& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;

    uint32_t count = 0;

    float width = 0.0f;
    const float scale = size / font.font_size;

    float x = 0.0f;
    uint32_t codepoint = 0;
    for (size_t i = 0; i < length;) {
        i += sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);

        if (codepoint == '\n') {
            x = 0.0f;
            continue;
        }

        auto it = font.glyphs.find(codepoint);
        if (it == font.glyphs.end()) {
            it = font.glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += (glyph.advance >> 6) * scale;
        width = std::max(width, x);

        if (width >= line_width) {
            break;
        }

        count++;
    }

    return count;
}

uint32_t sge::chars_fit_in_line_from_end(const Font& font, float size, const char* text, size_t length, float line_width) noexcept {
    ZoneScoped;

    uint32_t count = 0;

    float width = 0.0f;
    const float scale = size / font.font_size;

    float x = 0.0f;
    uint32_t codepoint = 0;
    for (int i = length; i > 0;) {
        i -= count_utf8_char_bytes_from_end(text, i);
        sge::utf8_codepoint_to_utf32(reinterpret_cast<const uint8_t*>(text) + i, codepoint);

        if (codepoint == '\n') {
            x = 0.0f;
            continue;
        }

        auto it = font.glyphs.find(codepoint);
        if (it == font.glyphs.end()) {
            it = font.glyphs.find(0);
        }
        
        const sge::Glyph& glyph = it->second;

        x += (glyph.advance >> 6) * scale;
        width = std::max(width, x);

        if (width >= line_width) {
            break;
        }

        count++;
    }

    return count;
}