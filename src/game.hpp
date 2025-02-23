#ifndef GAME_HPP
#define GAME_HPP

#pragma once

#include "engine/types/backend.hpp"
#include "engine/types/config.hpp"

namespace Game {
    bool Init(sge::types::RenderBackend backend, sge::types::AppConfig config);
    void Run();
    void Destroy();
};

#endif