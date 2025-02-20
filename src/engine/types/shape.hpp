#pragma once

#ifndef _ENGINE_TYPES_SHAPE_HPP_
#define _ENGINE_TYPES_SHAPE_HPP_

#include <cstdint>

namespace Shape {
    using Type = uint8_t;

    enum : Type {
        Rect = 0,
        Circle
    };
}

#endif