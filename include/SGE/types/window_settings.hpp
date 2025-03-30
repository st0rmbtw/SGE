#ifndef _SGE_TYPES_WINDOW_SETTINGS_HPP_
#define _SGE_TYPES_WINDOW_SETTINGS_HPP_

#include <stdint.h>

#include "../defines.hpp"

_SGE_BEGIN

struct WindowSettings {
    uint32_t width = 1280;
    uint32_t height = 720;
    bool fullscreen = false;
    bool hidden = false;
};

_SGE_END

#endif