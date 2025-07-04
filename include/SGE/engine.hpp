#ifndef _SGE_ENGINE_HPP_
#define _SGE_ENGINE_HPP_

#pragma once

#include <glm/vec2.hpp>

#include "types/backend.hpp"
#include "types/window_settings.hpp"
#include "types/cursor_mode.hpp"
#include "renderer/renderer.hpp"
#include "defines.hpp"

_SGE_BEGIN

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

struct EngineConfig {
    sge::WindowSettings window_settings;
    const char* pipeline_cache_path = DEFAULT_CACHE_DIR;
    bool cache_pipelines = true;
};

namespace Engine {
    using PreUpdateCallback = void (*)(void);
    using UpdateCallback = void (*)(void);
    using PostUpdateCallback = void (*)(void);
    using FixedUpdateCallback = void (*)(void);
    using RenderCallback = void (*)(void);
    using PostRenderCallback = void (*)(void);
    using DestroyCallback = void (*)(void);
    using LoadAssetsCallback = bool (*)(void);
    using WindowResizeCallback = void (*)(uint32_t width, uint32_t height, uint32_t scaled_width, uint32_t scaled_height);

    bool Init(sge::RenderBackend backend, const EngineConfig& config, LLGL::Extent2D& viewport);
    void SetPreUpdateCallback(PreUpdateCallback);
    void SetUpdateCallback(UpdateCallback);
    void SetPostUpdateCallback(PostUpdateCallback);
    void SetFixedUpdateCallback(FixedUpdateCallback);
    void SetFixedPostUpdateCallback(FixedUpdateCallback);
    void SetRenderCallback(RenderCallback);
    void SetPostRenderCallback(PostRenderCallback);
    void SetDestroyCallback(DestroyCallback);
    void SetWindowResizeCallback(WindowResizeCallback);
    void SetLoadAssetsCallback(LoadAssetsCallback);

    /*!
     *  @param[in] min_width The minimum width of the content
     *  area of the primary window, or -1 for any.
     *  @param[in] min_height The minimum height of the content
     *  area of the primary window, or -1 for any.
     */
    void SetWindowMinSize(int min_width, int min_height);

    /*!
     *  @param[in] max_width The maximum width of the content
     *  area of the primary window, or -1 for any.
     *  @param[in] max_height The maximum height of the content
     *  area of the primary window, or -1 for any.
     */
    void SetWindowMaxSize(int max_width, int max_height);

    void ShowWindow();
    void HideWindow();

    void SetCursorMode(CursorMode cursor_mode);

    uint32_t GetFrameCount();

    void Run();
    void Destroy();

    sge::Renderer& Renderer();
};

_SGE_END

#endif