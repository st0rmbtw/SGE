#include <SGE/utils/text.hpp>
#include <SGE/utils/utf8.hpp>
#include <SGE/profile.hpp>

glm::vec2 sge::calculate_text_bounds(const sge::Font& font, size_t length, const char* text, float size) {
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

        const sge::Glyph& glyph = font.glyphs.find(codepoint)->second;
        x += (glyph.advance >> 6) * scale;
        bounds.x = std::max(bounds.x, x);
        // bounds.y = std::max(bounds.y, (glyph.size.y + (glyph.size.y - glyph.bearing.y)) * scale);
    }

    return bounds;
}