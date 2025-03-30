#ifndef _SGE_TYPES_RENDER_LAYER_HPP_
#define _SGE_TYPES_RENDER_LAYER_HPP_

#pragma once

#include <stdint.h>

#include "../defines.hpp"

_SGE_BEGIN

enum class RenderLayer : uint8_t {
    Main = 0,
    World = 1
};

_SGE_END

#endif