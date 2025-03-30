#pragma once

#ifndef _SGE_TYPES_FONT_HPP_
#define _SGE_TYPES_FONT_HPP_

#include <unordered_map>
#include "texture.hpp"
#include "glyph.hpp"
#include "../defines.hpp"

_SGE_BEGIN

struct Font {
    std::unordered_map<uint32_t, Glyph> glyphs;
    Texture texture;
    float font_size;
    int16_t ascender;
};

_SGE_END

#endif