#pragma once

#ifndef _SGE_TYPES_GLYPH_HPP_
#define _SGE_TYPES_GLYPH_HPP_

#include <glm/vec2.hpp>

#include "../defines.hpp"

_SGE_BEGIN

namespace types {

struct Glyph {
    glm::ivec2 size;
    glm::vec2 tex_size;
    glm::ivec2 bearing;
    signed long advance;
    glm::vec2 texture_coords;
};

}

_SGE_END

#endif