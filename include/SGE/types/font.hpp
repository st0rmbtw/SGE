#pragma once

#ifndef SGE_TYPES_FONT_HPP_
#define SGE_TYPES_FONT_HPP_

#include <cstdlib>
#include <unordered_map>

#include <SGE/font_loader.hpp>
#include <SGE/log.hpp>

#include "texture.hpp"

namespace sge {

#if SGE_DEFAULT_FONT_ENABLED
namespace internal {

void InitDefaultFont(class sge::RenderContext& context);

} // namespace internal
#endif

struct GlyphDataVector {
    uint32_t partition_offset;
    uint32_t partition_count;
};

struct GlyphDataSDF {
    glm::vec2 tex_size;
    glm::vec2 texture_coords;
};

struct Glyph {
    union {
        GlyphDataSDF sdf;
        GlyphDataVector vector;
    } data;
    glm::ivec2 size;
    glm::ivec2 bearing;
    float advance;
};

struct FontVector {
    std::unordered_map<uint32_t, Glyph> glyphs;
    sge::Ref<LLGL::Buffer> curve_buffer;
    sge::Ref<LLGL::Buffer> partition_buffer;
    uint16_t units_per_em;
    int16_t ascender;
    int16_t descender;
};

struct Font {
    std::unordered_map<uint32_t, Glyph> glyphs;
    Texture texture;
    float font_size;
    float base_scale;
    int16_t ascender;
    int16_t descender;
};

#if SGE_DEFAULT_FONT_ENABLED

/**
 * @brief Returns the default vector font (JetBrains Mono Regular)
 */
const FontVector& GetDefaultFontVector();

/**
 * @brief Returns the default SDF font (JetBrains Mono Regular)
 */
const Font& GetDefaultFont();

#else // #if SGE_DEFAULT_FONT_ENABLED

/**
 * @brief Returns the default vector font (JetBrains Mono Regular)
 */
inline const FontVector& GetDefaultFontVector() {
    SGE_LOG_ERROR("To use the default font use must enable it with the `SGE_DEFAULT_FONT_ENABLED` option.");
    std::abort();
}

/**
 * @brief Returns the default SDF font (JetBrains Mono Regular)
 */
inline const Font& GetDefaultFont() {
    SGE_LOG_ERROR("To use the default font use must enable it with the `SGE_DEFAULT_FONT_ENABLED` option.");
    std::abort();
}

#endif // #if SGE_DEFAULT_FONT_ENABLED

// ================== Vector ==================
FontVector LoadVectorFontFromData(const sge::font_loader::FontDataVector& data, class sge::RenderContext& context);
FontVector LoadVectorFontFromBytes(std::span<const uint8_t> buffer, class sge::RenderContext& context);
FontVector LoadVectorFontFromFile(const std::string& path, class sge::RenderContext& context);

// ================== SDF ==================
Font LoadFontFromData(const sge::font_loader::FontDataSDF& data, class sge::RenderContext& context);
Font LoadFontFromBytes(std::span<const uint8_t> buffer, class sge::RenderContext& context);
Font LoadFontFromFile(const std::string& path, class sge::RenderContext& context);

} // namespace sge

#endif // SGE_TYPES_FONT_HPP_