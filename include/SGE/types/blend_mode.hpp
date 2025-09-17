#ifndef _SGE_TYPES_BLEND_MODE_HPP_
#define _SGE_TYPES_BLEND_MODE_HPP_

#pragma once

#include <cstdint>

namespace sge {

enum class BlendMode : uint8_t {
    AlphaBlend,
    Additive,
    Opaque,
    PremultipliedAlpha
};

}

#endif