#include <LLGL/Types.h>
#include <LLGL/Log.h>
#include <GLFW/glfw3.h>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/time/time.hpp>

#include "defines.hpp"

using namespace sge;

static struct EngineState {
    Renderer renderer;
    GLFWwindow *window = nullptr;
    Engine::PreUpdateCallback pre_update_callback = nullptr;
    Engine::UpdateCallback update_callback = nullptr;
    Engine::PostUpdateCallback post_update_callback = nullptr;
    Engine::FixedUpdateCallback fixed_update_callback = nullptr;
    Engine::FixedUpdateCallback fixed_post_update_callback = nullptr;
    Engine::RenderCallback render_callback = nullptr;
    Engine::PostRenderCallback post_render_callback = nullptr;
    Engine::DestroyCallback destroy_callback = nullptr;
    Engine::LoadAssetsCallback load_assets_callback = nullptr;
    Engine::WindowResizeCallback window_resize_callback = nullptr;

    LLGL::Extent2D window_size;
    glm::ivec2 window_min_size = glm::ivec2(-1, -1);
    glm::ivec2 window_max_size = glm::ivec2(-1, -1);

    uint32_t frame_count = 0;

    uint8_t window_samples = 1;
    bool window_iconified = false;
} state;

static void HandleKeyboardEvents(GLFWwindow* window, int key, int scancode, int action, int mods);
static void HandleMouseButtonEvents(GLFWwindow* window, int button, int action, int mods);
static void HandleMouseScrollEvents(GLFWwindow* window, double xoffset, double yoffset);
static void HandleCursorPosEvents(GLFWwindow* window, double xpos, double ypos);
static void HandleWindowResizeEvents(GLFWwindow* window, int width, int height);
static void HandleWindowIconifyCallback(GLFWwindow* window, int iconified);

static inline const char* glfwGetErrorString() {
    const char* description = nullptr;
    glfwGetError(&description);
    return description;
}

static GLFWwindow* CreateWindow(const WindowSettings& window_settings) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, window_settings.samples);
    glfwWindowHint(GLFW_RESIZABLE, window_settings.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* primary_monitor = window_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(window_settings.width, window_settings.height, window_settings.title, primary_monitor, nullptr);
    if (window == nullptr) {
        SGE_LOG_ERROR("Couldn't create a window: {}", glfwGetErrorString());
        return nullptr;
    }

    glfwSetKeyCallback(window, HandleKeyboardEvents);
    glfwSetMouseButtonCallback(window, HandleMouseButtonEvents);
    glfwSetScrollCallback(window, HandleMouseScrollEvents);
    glfwSetCursorPosCallback(window, HandleCursorPosEvents);
    glfwSetWindowSizeCallback(window, HandleWindowResizeEvents);
    glfwSetWindowIconifyCallback(window, HandleWindowIconifyCallback);

    return window;
}

static inline LLGL::Extent2D get_scaled_resolution(uint32_t width, uint32_t height) {
    float xscale;
    float yscale;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
    return LLGL::Extent2D(width * xscale, height * yscale);
}

void Engine::SetPreUpdateCallback(PreUpdateCallback callback) {
    state.pre_update_callback = callback;
}

void Engine::SetUpdateCallback(UpdateCallback callback) {
    state.update_callback = callback;
}

void Engine::SetPostUpdateCallback(PostUpdateCallback callback) {
    state.post_update_callback = callback;
}

void Engine::SetFixedUpdateCallback(FixedUpdateCallback callback) {
    state.fixed_update_callback = callback;
}

void Engine::SetFixedPostUpdateCallback(FixedUpdateCallback callback) {
    state.fixed_post_update_callback = callback;
}

void Engine::SetRenderCallback(RenderCallback callback) {
    state.render_callback = callback;
}

void Engine::SetPostRenderCallback(PostRenderCallback callback) {
    state.post_render_callback = callback;
}

void Engine::SetDestroyCallback(PostRenderCallback callback) {
    state.destroy_callback = callback;
}

void Engine::SetWindowResizeCallback(WindowResizeCallback callback) {
    state.window_resize_callback = callback;
}

void Engine::SetLoadAssetsCallback(LoadAssetsCallback callback) {
    state.load_assets_callback = callback;
}

void Engine::SetWindowMinSize(int min_width, int min_height) {
    state.window_min_size.x = min_width;
    state.window_min_size.y = min_height;
    glfwSetWindowSizeLimits(state.window, min_width, min_height, state.window_max_size.x, state.window_max_size.y);
}

void Engine::SetWindowMaxSize(int max_width, int max_height) {
    state.window_max_size.x = max_width;
    state.window_max_size.y = max_height;
    glfwSetWindowSizeLimits(state.window, state.window_min_size.x, state.window_min_size.y, max_width, max_height);
}

void Engine::ShowWindow() {
    glfwShowWindow(state.window);
}

void Engine::HideWindow() {
    glfwHideWindow(state.window);
}

void Engine::SetCursorMode(CursorMode cursor_mode) {
    glfwSetInputMode(state.window, GLFW_CURSOR, static_cast<int>(cursor_mode));
}

uint32_t Engine::GetFrameCount() {
    return state.frame_count;
}

bool Engine::Init(sge::RenderBackend backend, const EngineConfig& config, LLGL::Extent2D& output_viewport) {
    ZoneScoped;

#if SGE_PLATFORM_LINUX
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

    if (!glfwInit()) {
        SGE_LOG_ERROR("Couldn't initialize GLFW: {}", glfwGetErrorString());
        return false;
    }

    LLGL::Log::RegisterCallbackStd();

    if (!state.renderer.InitEngine(backend, config.cache_pipelines, config.pipeline_cache_path)) return false;

    const WindowSettings& window_settings = config.window_settings;

    LLGL::Extent2D window_size = LLGL::Extent2D(window_settings.width, window_settings.height);
    if (window_settings.fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window_size = LLGL::Extent2D(mode->width, mode->height);
    }

    GLFWwindow *window = CreateWindow(window_settings);
    if (window == nullptr) return false;

    state.window = window;
    state.window_size = window_size;

    output_viewport.width = window_size.width;
    output_viewport.height = window_size.height;

    SetCursorMode(window_settings.cursor_mode);

    const LLGL::Extent2D resolution = get_scaled_resolution(window_size.width, window_size.height);
    if (!state.renderer.Init(window, resolution, config.window_settings)) return false;

    state.window_samples = state.renderer.SwapChain()->GetSamples();

    if (state.load_assets_callback) {
        if (!state.load_assets_callback()) return false;
    }

    Time::SetFixedTimestepSeconds(1.0f / 60.0f);

    return true;
}

static inline bool IsDrawable() {
    return !state.window_iconified && (state.window_size.width >= state.window_samples && state.window_size.height >= state.window_samples);
}

void Engine::Run() {
    double prev_tick = glfwGetTime();

    double fixed_timer = 0;

    while (state.renderer.Surface()->ProcessEvents()) {
        MACOS_AUTORELEASEPOOL_OPEN
            const double current_tick = glfwGetTime();
            const double delta_time = (current_tick - prev_tick);
            prev_tick = current_tick;

            const delta_time_t dt(delta_time);
            Time::AdvanceBy(dt);

            if (state.pre_update_callback)
                state.pre_update_callback();

            int fixed_update_count = 0;

            fixed_timer += delta_time;
            while (fixed_timer >= Time::FixedDeltaSeconds()) {
                Time::AdvanceFixed();
                if (state.fixed_update_callback)
                    state.fixed_update_callback();
                fixed_timer -= Time::FixedDeltaSeconds();
                ++fixed_update_count;
            }

            if (state.update_callback)
                state.update_callback();

            for (int i = 0; i < fixed_update_count; ++i) {
                if (state.fixed_post_update_callback)
                    state.fixed_post_update_callback();
            }

            if (state.post_update_callback)
                state.post_update_callback();

            if (IsDrawable()) {
                if (state.render_callback) {
                    state.render_callback();
                    ++state.frame_count;
                }

                if (state.post_render_callback)
                    state.post_render_callback();
            }

            Input::Clear();
        MACOS_AUTORELEASEPOOL_CLOSE

        FrameMark;
    }
}

void Engine::Destroy() {
    if (state.renderer.CommandQueue()) {
        state.renderer.CommandQueue()->WaitIdle();
    }

    if (state.destroy_callback != nullptr) {
        state.destroy_callback();
    }

    if (state.renderer.Context()) {
        state.renderer.Terminate();
    }

    glfwTerminate();
}

sge::Renderer& Engine::Renderer() { return state.renderer; }

static void HandleKeyboardEvents(GLFWwindow*, int key, int, int action, int mods) {
    if (action == GLFW_PRESS) {
        Input::Press(static_cast<Key>(key), mods);
    } else if (action == GLFW_RELEASE) {
        Input::Release(static_cast<Key>(key), mods);
    }
}

static void HandleMouseButtonEvents(GLFWwindow*, int button, int action, int) {
    if (action == GLFW_PRESS) {
        Input::Press(static_cast<MouseButton>(button));
    } else if (action == GLFW_RELEASE) {
        Input::Release(static_cast<MouseButton>(button));
    }
}

static void HandleMouseScrollEvents(GLFWwindow*, double, double yoffset) {
    Input::PushMouseScrollEvent(yoffset);
}

static void HandleCursorPosEvents(GLFWwindow*, double xpos, double ypos) {
    const glm::uvec2 window_size = glm::uvec2(state.window_size.width, state.window_size.height);

    xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.x));
    ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.y));

    Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
}

static void HandleWindowResizeEvents(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) {
        state.window_size.width = 0;
        state.window_size.height = 0;
        return;
    }

    const LLGL::Extent2D resolution = get_scaled_resolution(width, height);

    state.renderer.ResizeBuffers(LLGL::Extent2D(resolution.width, resolution.height));

    state.window_size.width = width;
    state.window_size.height = height;

    if (state.window_resize_callback) {
        state.window_resize_callback(static_cast<uint32_t>(width), static_cast<uint32_t>(height), resolution.width, resolution.height);
    }

    state.render_callback();
}

static void HandleWindowIconifyCallback(GLFWwindow*, int iconified) {
    state.window_iconified = iconified == GLFW_TRUE;
}