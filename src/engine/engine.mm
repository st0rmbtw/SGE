#include <LLGL/Types.h>
#include <LLGL/Log.h>
#include <GLFW/glfw3.h>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/time/time.hpp>
#include <cstdlib>

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
    Engine::CleanupCallback cleanup_callback = nullptr;
    Engine::LoadAssetsCallback load_assets_callback = nullptr;
    Engine::WindowResizeCallback window_resize_callback = nullptr;

    LLGL::Extent2D window_size;
    glm::ivec2 window_min_size = glm::ivec2(-1, -1);
    glm::ivec2 window_max_size = glm::ivec2(-1, -1);

    uint32_t frame_count = 0;

    uint8_t window_samples = 1;
    bool window_iconified = false;
    bool running = false;
} state;

static void HandleKeyboardEvents(GLFWwindow* window, int key, int scancode, int action, int mods);
static void HandleMouseButtonEvents(GLFWwindow* window, int button, int action, int mods);
static void HandleMouseScrollEvents(GLFWwindow* window, double xoffset, double yoffset);
static void HandleCursorPosEvents(GLFWwindow* window, double xpos, double ypos);
static void HandleWindowResize(GLFWwindow* window, int width, int height);
static void HandleFramebufferResize(GLFWwindow* window, int width, int height);
static void HandleWindowIconify(GLFWwindow* window, int iconified);
static void HandleCharacterCallback(GLFWwindow* window, uint32_t codepoint);

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

void Engine::SetCleanupCallback(CleanupCallback callback) {
    state.cleanup_callback = callback;
}

void Engine::SetWindowResizeCallback(WindowResizeCallback callback) {
    state.window_resize_callback = callback;
}

void Engine::SetLoadAssetsCallback(LoadAssetsCallback callback) {
    state.load_assets_callback = callback;
}

uint32_t Engine::GetFrameCount() {
    return state.frame_count;
}

bool Engine::Init(sge::RenderBackend backend, const EngineConfig& config, LLGL::Extent2D& output_viewport) {
    ZoneScoped;

    if (!state.renderer.Init(backend, config.cache_pipelines, config.pipeline_cache_path)) return false;

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

    return true;
}

static inline bool IsDrawable() {
    return !state.window_iconified && (state.window_size.width >= state.window_samples && state.window_size.height >= state.window_samples);
}

void Engine::Destroy() {
    if (state.renderer.CommandQueue()) {
        state.renderer.CommandQueue()->WaitIdle();
    }

    if (state.cleanup_callback != nullptr) {
        state.cleanup_callback();
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

static void HandleFramebufferResize(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    state.renderer.ResizeBuffers(LLGL::Extent2D(width, height));
}

static void HandleWindowResize(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) {
        state.window_size.width = 0;
        state.window_size.height = 0;
        return;
    }

    state.window_size.width = width;
    state.window_size.height = height;

    if (state.window_resize_callback) {
        const LLGL::Extent2D resolution = get_scaled_resolution(width, height);
        state.window_resize_callback(static_cast<uint32_t>(width), static_cast<uint32_t>(height), resolution.width, resolution.height);
    }

    state.render_callback();
}

static void HandleWindowIconify(GLFWwindow*, int iconified) {
    state.window_iconified = iconified == GLFW_TRUE;
}

static void HandleCharacterCallback(GLFWwindow*, uint32_t codepoint) {
    Input::AddCodePoint(codepoint);
}




IEngine::IEngine() {
#if SGE_PLATFORM_LINUX
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

    if (!glfwInit()) {
        SGE_LOG_ERROR("Couldn't initialize GLFW: {}", glfwGetErrorString());
        std::abort();
    }

    LLGL::Log::RegisterCallbackStd();
    Time::SetFixedTimestepSeconds(1.0f / 60.0f);
}

void IEngine::Run() {
    double prev_tick = glfwGetTime();

    double fixed_timer = 0;

    m_running = true;

    while (m_running) {
        if (ShouldStop()) {
            break;
        }

        MACOS_AUTORELEASEPOOL_OPEN
            const double current_tick = glfwGetTime();
            const double delta_time = (current_tick - prev_tick);
            prev_tick = current_tick;

            const delta_time_t dt(delta_time);
            Time::AdvanceBy(dt);

            OnPreUpdate();

            int fixed_update_count = 0;

            fixed_timer += delta_time;
            while (fixed_timer >= Time::FixedDeltaSeconds()) {
                Time::AdvanceFixed();
                OnFixedUpdate();
                fixed_timer -= Time::FixedDeltaSeconds();
                ++fixed_update_count;
            }

            OnUpdate();

            for (int i = 0; i < fixed_update_count; ++i) {
                OnFixedPostUpdate();
            }

            OnPostUpdate();

            if (IsDrawable()) {
                OnRender();
                OnPostRender();
                ++m_frame_count;
            }

            Input::Clear();
        MACOS_AUTORELEASEPOOL_CLOSE

        FrameMark;
    }

    m_running = false;
}