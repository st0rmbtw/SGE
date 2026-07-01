#if SGE_DEFAULT_FONT_ENABLED

#include <SGE/types/font.hpp>
#include <SGE/renderer/context.hpp>

#include "default_font.hpp"

namespace {

sge::FontVector g_font_vector;
sge::Font g_font_sdf;

} // namespace

void sge::internal::InitDefaultFont(sge::RenderContext& context) {
    std::unordered_map<uint32_t, Glyph> glyphs;
    for (EmbeddedFontGlyph glyph : FONT_GLYPHS) {
        glyphs[glyph.character] = sge::Glyph {
            .data = {
                .sdf = {
                    .tex_size = glyph.tex_size,
                    .texture_coords = glyph.texture_coords,
                }
            },
            .size = glyph.size,
            .bearing = glyph.bearing,
            .advance = glyph.advance,
        };
    }

    sge::TextureConfig textureConfig;
    textureConfig.debugName = "Default Font Texture";
    textureConfig.textureType = LLGL::TextureType::Texture2D;
    textureConfig.extent.width = FONT_META_DATA.texture_width;
    textureConfig.extent.height = FONT_META_DATA.texture_height;
    textureConfig.extent.depth = 1;
    textureConfig.sampler = context.GetLinearSampler();

    LLGL::ImageView imageView;
    imageView.dataType = LLGL::DataType::UInt8;
    imageView.format = LLGL::ImageFormat::R;
    imageView.dataSize = static_cast<size_t>(FONT_META_DATA.texture_width) * static_cast<size_t>(FONT_META_DATA.texture_height);
    imageView.data = FONT_TEXTURE_DATA;

    g_font_sdf = sge::Font {
        .glyphs = std::move(glyphs),
        .texture = context.CreateTexture(textureConfig, &imageView),
        .font_size = FONT_META_DATA.font_size,
        .max_ascent = FONT_META_DATA.max_ascent,
        .max_descent = FONT_META_DATA.max_descent,
        .ascender = static_cast<int16_t>(FONT_META_DATA.ascender >> 6),
    };
}

const sge::FontVector& sge::GetDefaultFontVector() {
    return g_font_vector;
}

const sge::Font& sge::GetDefaultFont() {
    return g_font_sdf;
}

#endif