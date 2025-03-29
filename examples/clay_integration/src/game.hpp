#ifndef GAME_HPP
#define GAME_HPP

#pragma once

#include <SGE/types/backend.hpp>
#include <SGE/types/config.hpp>

namespace Game {
    bool Init(sge::types::RenderBackend backend, sge::types::AppConfig config);
    void Run();
    void Destroy();
};

#endif