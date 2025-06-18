#ifndef _SGE_TYPES_WINDOW_SETTINGS_HPP_
#define _SGE_TYPES_WINDOW_SETTINGS_HPP_

#include <stdint.h>

#include <SGE/types/cursor_mode.hpp>
#include "../defines.hpp"

_SGE_BEGIN

struct WindowSettings {
    const char* title = "App";
    uint32_t width = 1280;
    uint32_t height = 720;
    CursorMode cursor_mode = CursorMode::Normal;
    uint8_t samples = 4;
    bool fullscreen = false;
    bool hidden = false;
    bool vsync = false;
};

_SGE_END

#endif