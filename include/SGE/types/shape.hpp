#pragma once

#ifndef _SGE_TYPES_SHAPE_HPP_
#define _SGE_TYPES_SHAPE_HPP_

#include <cstdint>

#include "../defines.hpp"

_SGE_BEGIN

namespace Shape {
    using Type = uint8_t;

    enum : Type {
        Rect = 0,
        Circle,
        Arc
    };
}

_SGE_END

#endif