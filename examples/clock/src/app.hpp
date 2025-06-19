#ifndef APP_HPP
#define APP_HPP

#pragma once

#include <SGE/types/backend.hpp>

struct AppConfig {
    bool vsync = false;
    bool fullscreen = false;
    uint8_t samples = 4;
};

namespace App {
    bool Init(sge::RenderBackend backend, AppConfig config);
    void Run();
    void Destroy();
};

#endif