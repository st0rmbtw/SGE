#include <SGE/utils/text.hpp>
#include <SGE/utils/utf8.hpp>
#include <SGE/profile.hpp>

glm::vec2 sge::calculate_text_bounds(const sge::Font& font, size_t length, const char* text, float size) {
    ZoneScoped;

    auto bounds = glm::vec2(0.0f);
    float prev_x = 0.0f;

    const float scale = size / font.font_size;

    for (size_t i = 0; i < length;) {
        const uint32_t ch = sge::next_utf8_codepoint(text, i);

        if (ch == '\n') {
            bounds.y += size;
            prev_x = 0.0f;
            continue;
        }

        const sge::Glyph& glyph = font.glyphs.find(ch)->second;
        prev_x += (glyph.advance >> 6) * scale;
        bounds.x = std::max(prev_x, bounds.x);
    }

    return bounds;
}