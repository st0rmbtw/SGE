#pragma once

#ifndef _SGE_TYPES_FONT_HPP_
#define _SGE_TYPES_FONT_HPP_

#include <unordered_map>

#include "texture.hpp"
#include "glyph.hpp"

namespace sge {

#if SGE_DEFAULT_FONT_ENABLED
namespace internal {
    
void InitDefaultFont(class sge::RenderContext& context);

} // namespace internal
#endif

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
 * @brief Returns the default font (JetBrains Mono Regular)
 */
const Font& GetDefaultFont();
#endif

} // namespace sge

#endif