#ifndef UI_HPP
#define UI_HPP

#pragma once

#include <utility>

#include "renderer/camera.h"

namespace UI {
    void Init();
    void Update();
    void Draw(const Camera& camera);
};

#endif