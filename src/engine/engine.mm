#include <LLGL/Types.h>
#include <LLGL/Log.h>
#include <GLFW/glfw3.h>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/time/time.hpp>
#include <cstdlib>
#include <memory>

#include "defines.hpp"
#include "utils.hpp"

using namespace sge;

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

    m_context = std::make_shared<RenderContext>();
}

IEngine::~IEngine() {
    glfwTerminate();
}

void IEngine::Run() {
    double prev_tick = glfwGetTime();

    double fixed_timer = 0;

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

            for (auto& [glfw, window] : m_window_map) {
                LLGL::Extent2D size = window->GetContentSize();

                if (!window->IsMinimized() && (size.width >= window->GetSamples() && size.height >= window->GetSamples())) {
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
}

std::expected<GlfwWindow*, const char*> IEngine::CreateWindow(const WindowSettings& window_settings) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, window_settings.samples);
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

    std::shared_ptr<GlfwWindow> instance = std::make_shared<GlfwWindow>(window, LLGL::Extent2D(width, height), position, window_settings.samples, window_settings.fullscreen);
    instance->Listen(*this);

    m_window_map.try_emplace(window, instance);
    return instance.get();
}