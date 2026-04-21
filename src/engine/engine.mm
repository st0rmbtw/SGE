#include <LLGL/Types.h>
#include <LLGL/Log.h>
#include <GLFW/glfw3.h>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/time/time.hpp>
#include <memory>

#include "defines.hpp"
#include "utils.hpp"

using namespace sge;

bool IEngine::Init() {
    if (m_initialized) {
        SGE_LOG_ERROR("Engine::Init was called twice!");
        std::abort();
    }

#if SGE_PLATFORM_LINUX
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

    if (!glfwInit()) {
        SGE_LOG_ERROR("Couldn't initialize GLFW: {}", glfwGetErrorString());
        return false;
    }

    LLGL::Log::RegisterCallbackStd();
    Time::SetFixedTimestepSeconds(1.0f / 60.0f);

    m_context = std::make_shared<RenderContext>();

    if (!OnInit())
        return false;

    m_initialized = true;
    
    return true;
}

IEngine::~IEngine() {
    if (m_context)
        m_context->Destroy();

    glfwTerminate();
}

void IEngine::Run() {
    if (!m_initialized) {
        SGE_LOG_ERROR("Engine is not initialized. Did you forget to call `IEngine::Init` function?");
        return;
    }

    double prev_tick = glfwGetTime();

    m_running = true;

    while (m_running) {
        glfwPollEvents();

        for (auto it = m_window_map.begin(); it != m_window_map.end();) {
            const auto& window = it->second;
            if (window->ShouldBeClosed()) {
                OnWindowDestroy(*window);
                m_context->UnregisterWindow(*window);
                it = m_window_map.erase(it);
            } else {
                ++it;
            }
        }

        MACOS_AUTORELEASEPOOL_OPEN
            const double current_tick = glfwGetTime();
            double delta_time = (current_tick - prev_tick);
            prev_tick = current_tick;

            if (delta_time > 0.25)
                delta_time = 0.25;

            const delta_time_t dt(delta_time);
            Time::AdvanceBy(dt);

            OnPreFixedUpdate();
            while (Time::Overstep() >= Time::FixedDeltaSeconds()) {
                Time::AdvanceFixed();
                OnFixedUpdate();
            }
            OnPostFixedUpdate();

            OnPreUpdate();
            OnUpdate();
            OnPostUpdate();

            for (auto& [glfw, window] : m_window_map) {
                const LLGL::Extent2D size = window->GetContentSize();

                if (!window->IsMinimized() && (size.width > 0 && size.height > 0)) {
                    OnRender(window);
                    OnPostRender(window);
                }
            }

            ++m_frame_count;

            Input::Clear();
        MACOS_AUTORELEASEPOOL_CLOSE

        FrameMark;
    }

    m_running = false;

    if (m_context->IsInitialized()) {
        m_context->GetCommandQueue()->WaitIdle();
    }
}

std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> IEngine::CreateWindow(const WindowSettings& window_settings) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_RESIZABLE, window_settings.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* primary_monitor = window_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(window_settings.width, window_settings.height, window_settings.title, primary_monitor, nullptr);
    if (window == nullptr) {
        return std::unexpected(glfwGetErrorString());
    }

    glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(window_settings.cursor_mode));

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::ivec2 position;
    glfwGetWindowPos(window, &position.x, &position.y);

    const uint8_t vsync_interval = window_settings.vsync;

    std::shared_ptr<GlfwWindow> instance = std::make_shared<GlfwWindow>(window, LLGL::Extent2D(width, height), position, window_settings.cursor_mode, window_settings.samples, vsync_interval, window_settings.fullscreen);
    instance->Listen(*this);

    m_window_map.try_emplace(window, instance);
    return instance;
}
