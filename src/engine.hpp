#ifndef ENGINE_HPP
#define ENGINE_HPP

#pragma once

#include "types/backend.hpp"

namespace Engine {
    using PreUpdateCallback = void (*)(void);
    using UpdateCallback = void (*)(void);
    using PostUpdateCallback = void (*)(void);
    using FixedUpdateCallback = void (*)(void);
    using RenderCallback = void (*)(void);
    using PostRenderCallback = void (*)(void);
    using LoadAssetsCallback = bool (*)(void);
    using WindowResizeCallback = void (*)(uint32_t width, uint32_t height);

    bool Init(RenderBackend backend, bool vsync, bool fullscreen, uint32_t window_width, uint32_t window_height, bool window_hidden = false);
    void SetPreUpdateCallback(PreUpdateCallback);
    void SetUpdateCallback(UpdateCallback);
    void SetPostUpdateCallback(PostUpdateCallback);
    void SetFixedUpdateCallback(FixedUpdateCallback);
    void SetRenderCallback(RenderCallback);
    void SetPostRenderCallback(PostRenderCallback);
    void SetWindowResizeCallback(WindowResizeCallback);
    void SetLoadAssetsCallback(LoadAssetsCallback);

    void SetWindowMinSize(uint32_t min_width, uint32_t min_height);
    void ShowWindow();
    void HideWindow();

    void Run();
    void Destroy();
};

#endif