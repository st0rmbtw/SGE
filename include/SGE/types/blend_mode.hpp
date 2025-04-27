#ifndef _SGE_TYPES_BLEND_MODE_HPP_
#define _SGE_TYPES_BLEND_MODE_HPP_

#pragma once

#include <cstdint>
#include <SGE/defines.hpp>

_SGE_BEGIN

enum class BlendMode : uint8_t {
    AlphaBlend,
    Additive,
    Opaque,
    PremultipliedAlpha
};

_SGE_END

#endif