#pragma once

#ifndef TYPES_SHAPE
#define TYPES_SHAPE

#include <stdint.h>

namespace Shape {
    using Type = uint8_t;

    enum : Type {
        Rect = 0,
        Circle
    };
}

#endif