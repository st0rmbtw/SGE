#include <SGE/renderer/batch.hpp>
#include <SGE/utils.hpp>

#include <tracy/Tracy.hpp>

using namespace sge;

uint32_t Batch::DrawAtlasSprite(const TextureAtlasSprite& sprite, struct Order custom_order) {
    const sge::Rect& rect = sprite.atlas().get_rect(sprite.index());

    glm::vec4 uv_offset_scale = glm::vec4(
        rect.min.x / sprite.atlas().texture().width(),
        rect.min.y / sprite.atlas().texture().height(),
        rect.size().x / sprite.atlas().texture().width(),
        rect.size().y / sprite.atlas().texture().height()
    );

    if (sprite.flip_x()) {
        uv_offset_scale.x += uv_offset_scale.z;
        uv_offset_scale.z *= -1.0f;
    }

    if (sprite.flip_y()) {
        uv_offset_scale.y += uv_offset_scale.w;
        uv_offset_scale.w *= -1.0f;
    }

    return AddSpriteDrawCommand(sprite, uv_offset_scale, sprite.atlas().texture(), custom_order);
}

uint32_t Batch::DrawText(const RichTextSection* sections, size_t size, const glm::vec2& position, const Font& font, struct Order custom_order) {
    float x = position.x;
    float y = position.y;

    const uint32_t order = m_order_mode
        ? m_global_order.value + std::max(custom_order.value, 0)
        : (custom_order.value >= 0 ? custom_order.value : m_order);

    custom_order.advance |= m_global_order.advance;
        
    if (custom_order.advance) m_order = std::max(m_order, order + 1);

    for (size_t i = 0; i < size; ++i) {
        const RichTextSection section = sections[i];
        const char* str = section.text.data();
        const size_t length = section.text.size();
        const float scale = section.size / font.font_size;

        const glm::vec3 color = section.color.to_vec3();

        for (size_t i = 0; i < length;) {
            const uint32_t c = next_utf8_codepoint(str, i);

            if (c == '\n') {
                y += section.size;
                x = position.x;
                continue;
            }

            const Glyph& ch = font.glyphs.find(c)->second;

            if (c == ' ') {
                x += (ch.advance >> 6) * scale;
                continue;
            }

            const float xpos = x + ch.bearing.x * scale; // ⌄ Make the origin at the top left corner
            const float ypos = y - ch.bearing.y * scale + section.size - font.ascender * scale;
            const glm::vec2 pos = glm::vec2(xpos, ypos);
            const glm::vec2 size = glm::vec2(ch.size) * scale;

            AddGlyphDrawCommand(internal::DrawCommandGlyph {
                .texture = font.texture,
                .color = color,
                .pos = pos,
                .size = size,
                .tex_size = ch.tex_size,
                .tex_uv = ch.texture_coords,
            });

            x += (ch.advance >> 6) * scale;
        }
    }

    return order;
}

uint32_t Batch::AddSpriteDrawCommand(const BaseSprite& sprite, const glm::vec4& uv_offset_scale, const Texture& texture, struct Order custom_order) {
    const uint32_t order = m_order_mode
        ? m_global_order.value + std::max(custom_order.value, 0)
        : (custom_order.value >= 0 ? custom_order.value : m_order);

    custom_order.advance |= m_global_order.advance;
    
    if (custom_order.advance) {
        m_order = std::max(m_order, order + 1);
    }

    const internal::DrawCommandSprite draw_command = internal::DrawCommandSprite {
        .texture = texture,
        .rotation = sprite.rotation(),
        .uv_offset_scale = uv_offset_scale,
        .color = sprite.color().to_vec4(),
        .outline_color = sprite.outline_color().to_vec4(),
        .position = glm::vec3(sprite.position(), sprite.z()),
        .size = sprite.size(),
        .offset = sprite.anchor().to_vec2(),
        .outline_thickness = sprite.outline_thickness(),
        .ignore_camera_zoom = sprite.ignore_camera_zoom(),
        .depth_enabled = false
    };

    m_draw_commands.emplace_back(draw_command, m_sprite_count, m_order, m_blend_mode);

    ++m_sprite_count;

    return order;
}

uint32_t Batch::AddNinePatchDrawCommand(const NinePatch& ninepatch, const glm::vec4& uv_offset_scale, struct Order custom_order) {
    const uint32_t order = m_order_mode
        ? m_global_order.value + std::max(custom_order.value, 0)
        : (custom_order.value >= 0 ? custom_order.value : m_order);

    custom_order.advance |= m_global_order.advance;
    
    if (custom_order.advance) {
        m_order = std::max(m_order, order + 1);
    }

    const internal::DrawCommandNinePatch draw_command = internal::DrawCommandNinePatch {
        .texture = ninepatch.texture(),
        .rotation = ninepatch.rotation(),
        .uv_offset_scale = uv_offset_scale,
        .color = ninepatch.color().to_vec4(),
        .margin = ninepatch.margin(),
        .position = ninepatch.position(),
        .size = ninepatch.size(),
        .offset = ninepatch.anchor().to_vec2(),
        .source_size = ninepatch.texture().size(),
        .output_size = ninepatch.size(),
    };
    
    m_draw_commands.emplace_back(draw_command, m_ninepatch_count, m_order, m_blend_mode);

    ++m_ninepatch_count;

    return order;
}

void Batch::AddGlyphDrawCommand(const internal::DrawCommandGlyph& command) {
    m_draw_commands.emplace_back(command, m_glyph_count, m_order, m_blend_mode);
    
    ++m_glyph_count;
}

uint32_t Batch::DrawShape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const sge::LinearRgba& color, const sge::LinearRgba& border_color, float border_thickness, glm::vec4 border_radius, Anchor anchor, struct Order custom_order) {
    const uint32_t order = m_order_mode
        ? m_global_order.value + std::max(custom_order.value, 0)
        : (custom_order.value >= 0 ? custom_order.value : m_order);

    custom_order.advance |= m_global_order.advance;
    
    if (custom_order.advance) {
        m_order = std::max(m_order, order + 1);
    }

    internal::DrawCommandShape command = {
        .position = position,
        .size = size,
        .offset = anchor.to_vec2(),
        .color = color,
        .border_color = border_color,
        .border_radius = border_radius,
        .border_thickness = border_thickness,
        .shape = shape,
    };

    m_draw_commands.emplace_back(command, m_shape_count, m_order, m_blend_mode);

    ++m_shape_count;

    return order;
}