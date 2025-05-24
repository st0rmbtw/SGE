#ifndef _SGE_TYPES_WINDOW_SETTINGS_HPP_
#define _SGE_TYPES_WINDOW_SETTINGS_HPP_

#include <stdint.h>

#include "../defines.hpp"

_SGE_BEGIN

struct WindowSettings {
    uint32_t width = 1280;
    uint32_t height = 720;
    uint8_t samples = 4;
    bool fullscreen = false;
    bool hidden = false;
    bool vsync = false;
};

_SGE_END

#endif