#pragma once

#ifndef _SGE_TYPES_FONT_HPP_
#define _SGE_TYPES_FONT_HPP_

#include <string>
#include <unordered_map>

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
    int64_t advance;
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
    float max_ascent;
    float max_descent;
    int16_t ascender;
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

#include <cstdlib>
#include <SGE/log.hpp>

/**
 * @brief Returns the default vector font (JetBrains Mono Regular)
 */
const FontVector& GetDefaultFontVector() {
    SGE_LOG_ERROR("To use the default font use must enable it with the `SGE_DEFAULT_FONT_ENABLED` option.");
    std::abort();
}

/**
 * @brief Returns the default SDF font (JetBrains Mono Regular)
 */
const Font& GetDefaultFont() {
    SGE_LOG_ERROR("To use the default font use must enable it with the `SGE_DEFAULT_FONT_ENABLED` option.");
    std::abort();
}

#endif // #if SGE_DEFAULT_FONT_ENABLED

FontVector LoadFontVector(const std::string& path, class sge::RenderContext& context);
Font LoadFont(const std::string& path, class sge::RenderContext& context);

} // namespace sge

#endif