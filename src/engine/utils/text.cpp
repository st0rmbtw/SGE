#include <SGE/utils/text.hpp>
#include <SGE/utils/utf8.hpp>
#include <SGE/profile.hpp>

glm::vec2 sge::calculate_text_bounds(const sge::Font& font, size_t length, const char* text, float size) {
    ZoneScoped;

    auto bounds = glm::vec2(0.0f);
    float x = 0.0f;

    const float scale = size / font.font_size;

    bounds.y = (font.max_ascent + font.max_descent) * scale;

    for (size_t i = 0; i < length;) {
        const uint32_t ch = sge::next_utf8_codepoint(text, i);

        if (ch == '\n') {
            bounds.y += size;
            x = 0.0f;
            continue;
        }

        const sge::Glyph& glyph = font.glyphs.find(ch)->second;
        x += (glyph.advance >> 6) * scale;
        bounds.x = std::max(bounds.x, x);
        // bounds.y = std::max(bounds.y, (glyph.size.y + (glyph.size.y - glyph.bearing.y)) * scale);
    }

    return bounds;
}