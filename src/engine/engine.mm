#include <LLGL/Types.h>
#include <LLGL/Log.h>
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>

#include <SGE/time/time.hpp>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>

#include "defines.hpp"

using namespace sge;

static struct EngineState {
    Renderer renderer;
    GLFWwindow *window = nullptr;
    bool minimized = false;
    uint32_t window_width;
    uint32_t window_height;
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
} state;

static void default_callback() {}
static void default_window_resize_callback(uint32_t, uint32_t, uint32_t, uint32_t) {}
static bool default_load_assets_callback() { return true; }

static void handle_keyboard_events(GLFWwindow* window, int key, int scancode, int action, int mods);
static void handle_mouse_button_events(GLFWwindow* window, int button, int action, int mods);
static void handle_mouse_scroll_events(GLFWwindow* window, double xoffset, double yoffset);
static void handle_cursor_pos_events(GLFWwindow* window, double xpos, double ypos);
static void handle_window_resize_events(GLFWwindow* window, int width, int height);
static void handle_window_iconify_callback(GLFWwindow* window, int iconified);

static inline const char* glfwGetErrorString() {
    const char* description = nullptr;
    glfwGetError(&description);
    return description;
}

static GLFWwindow* create_window(LLGL::Extent2D size, bool fullscreen, bool hidden, uint8_t samples) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, samples);

    GLFWmonitor* primary_monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(size.width, size.height, "AAA", primary_monitor, nullptr);
    if (window == nullptr) {
        SGE_LOG_ERROR("Couldn't create a window: %s", glfwGetErrorString());
        return nullptr;
    }

    glfwSetKeyCallback(window, handle_keyboard_events);
    glfwSetMouseButtonCallback(window, handle_mouse_button_events);
    glfwSetScrollCallback(window, handle_mouse_scroll_events);
    glfwSetCursorPosCallback(window, handle_cursor_pos_events);
    glfwSetWindowSizeCallback(window, handle_window_resize_events);
    glfwSetWindowIconifyCallback(window, handle_window_iconify_callback);

    return window;
}

static inline LLGL::Extent2D get_scaled_resolution(uint32_t width, uint32_t height) {
    float xscale;
    float yscale;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
    return LLGL::Extent2D(width * xscale, height * yscale);
}

void Engine::SetPreUpdateCallback(PreUpdateCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.pre_update_callback = callback;
}

void Engine::SetUpdateCallback(UpdateCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.update_callback = callback;
}

void Engine::SetPostUpdateCallback(PostUpdateCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.post_update_callback = callback;
}

void Engine::SetFixedUpdateCallback(FixedUpdateCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.fixed_update_callback = callback;
}

void Engine::SetFixedPostUpdateCallback(FixedUpdateCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.fixed_post_update_callback = callback;
}

void Engine::SetRenderCallback(RenderCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.render_callback = callback;
}

void Engine::SetPostRenderCallback(PostRenderCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.post_render_callback = callback;
}

void Engine::SetDestroyCallback(PostRenderCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.destroy_callback = callback;
}

void Engine::SetWindowResizeCallback(WindowResizeCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.window_resize_callback = callback;
}

void Engine::SetLoadAssetsCallback(LoadAssetsCallback callback) {
    SGE_ASSERT(callback != nullptr, "Pointer to the callback must not be null.");
    state.load_assets_callback = callback;
}

void Engine::SetWindowMinSize(uint32_t min_width, uint32_t min_height) {
    glfwSetWindowSizeLimits(state.window, min_width, min_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
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

bool Engine::Init(sge::RenderBackend backend, sge::WindowSettings settings, LLGL::Extent2D& output_viewport) {
    ZoneScopedN("Engine::Init");

    if (state.pre_update_callback == nullptr) state.pre_update_callback = default_callback;
    if (state.update_callback == nullptr) state.update_callback = default_callback;
    if (state.post_update_callback == nullptr) state.post_update_callback = default_callback;
    if (state.fixed_update_callback == nullptr) state.fixed_update_callback = default_callback;
    if (state.fixed_post_update_callback == nullptr) state.fixed_post_update_callback = default_callback;
    if (state.render_callback == nullptr) state.render_callback = default_callback;
    if (state.post_render_callback == nullptr) state.post_render_callback = default_callback;
    if (state.destroy_callback == nullptr) state.destroy_callback = default_callback;
    if (state.window_resize_callback == nullptr) state.window_resize_callback = default_window_resize_callback;
    if (state.load_assets_callback == nullptr) state.load_assets_callback = default_load_assets_callback;

#if SGE_PLATFORM_LINUX
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

    if (!glfwInit()) {
        SGE_LOG_ERROR("Couldn't initialize GLFW: %s", glfwGetErrorString());
        return false;
    }

    LLGL::Log::RegisterCallbackStd();

    if (!state.renderer.InitEngine(backend, false)) return false;

    LLGL::Extent2D window_size = LLGL::Extent2D(settings.width, settings.height);
    if (settings.fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window_size = LLGL::Extent2D(mode->width, mode->height);
    }

    GLFWwindow *window = create_window(window_size, settings.fullscreen, settings.hidden, settings.samples);
    if (window == nullptr) return false;

    state.window = window;
    state.window_width = window_size.width;
    state.window_height = window_size.height;

    output_viewport.width = window_size.width;
    output_viewport.height = window_size.height;

    const LLGL::Extent2D resolution = get_scaled_resolution(window_size.width, window_size.height);
    if (!state.renderer.Init(window, resolution, settings)) return false;

    if (!state.load_assets_callback()) return false;

    Time::SetFixedTimestepSeconds(1.0f / 60.0f);

    return true;
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

            state.pre_update_callback();

            int fixed_update_count = 0;

            fixed_timer += delta_time;
            while (fixed_timer >= Time::FixedDeltaSeconds()) {
                Time::AdvanceFixed();
                state.fixed_update_callback();
                fixed_timer -= Time::FixedDeltaSeconds();
                ++fixed_update_count;
            }

            state.update_callback();

            for (int i = 0; i < fixed_update_count; ++i) {
                state.fixed_post_update_callback();
            }

            state.post_update_callback();

            if (!state.minimized) {
                state.render_callback();
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

    state.destroy_callback();

    if (state.renderer.Context()) {
        state.renderer.Terminate();
    }

    glfwTerminate();
}

sge::Renderer& Engine::Renderer() { return state.renderer; }

static void handle_keyboard_events(GLFWwindow*, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
        Input::Press(static_cast<Key>(key));
    } else if (action == GLFW_RELEASE) {
        Input::Release(static_cast<Key>(key));
    }
}

static void handle_mouse_button_events(GLFWwindow*, int button, int action, int) {
    if (action == GLFW_PRESS) {
        Input::Press(static_cast<MouseButton>(button));
    } else if (action == GLFW_RELEASE) {
        Input::Release(static_cast<MouseButton>(button));
    }
}

static void handle_mouse_scroll_events(GLFWwindow*, double, double yoffset) {
    Input::PushMouseScrollEvent(yoffset);
}

static void handle_cursor_pos_events(GLFWwindow*, double xpos, double ypos) {
    const glm::uvec2 window_size = glm::uvec2(state.window_width, state.window_height);

    xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.x));
    ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.y));

    Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
}

static void handle_window_resize_events(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) {
        state.minimized = true;
        return;
    }

    state.minimized = false;

    const LLGL::Extent2D resolution = get_scaled_resolution(width, height);

    // state.renderer.ResizeBuffers(LLGL::Extent2D(resolution.width * 4, resolution.height * 4));
    state.renderer.ResizeBuffers(LLGL::Extent2D(resolution.width, resolution.height));

    state.window_width = width;
    state.window_height = height;

    state.window_resize_callback(static_cast<uint32_t>(width), static_cast<uint32_t>(height), resolution.width, resolution.height);

    state.render_callback();
}

static void handle_window_iconify_callback(GLFWwindow*, int iconified) {
    state.minimized = iconified;
}