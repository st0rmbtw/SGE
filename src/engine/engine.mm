#include <memory>
#include <random>

#include <GLFW/glfw3.h>

#include <LLGL/Log.h>
#include <LLGL/Types.h>

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/profile.hpp>
#include <SGE/time/time.hpp>
#include <SGE/utils/random.hpp>
#include <SGE/window_manager.hpp>

#include "defines.hpp"
#include "utils.hpp"

bool sge::IEngine::Init() {
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

    std::random_device rd;
    Random::Seed(rd());

    m_initialized = true;
    
    return true;
}

sge::IEngine::~IEngine() {
    if (m_context)
        m_context->Destroy();

    glfwTerminate();
}

void sge::IEngine::Run() {
    if (!m_initialized) {
        SGE_LOG_ERROR("Engine is not initialized. Did you forget to call `IEngine::Init` function?");
        return;
    }

    m_prev_tick = glfwGetTime();

    m_running = true;

    while (m_running) {
        glfwPollEvents();

        auto& window_map = WindowManager::GetWindowMap();

        for (auto it = window_map.begin(); it != window_map.end();) {
            const auto& window = it->second;
            if (window->ShouldBeClosed()) {
                if (m_exit_on_main_window_destroy && window->GetID() == 0) {
                    Stop();
                }

                OnWindowDestroy(*window);
                m_context->UnregisterWindow(*window);
                it = window_map.erase(it);
            } else {
                ++it;
            }
        }

        MACOS_AUTORELEASEPOOL_OPEN
            Update();
            for (const auto& [glfw, window] : window_map) {
                Render(window);
            }
        MACOS_AUTORELEASEPOOL_CLOSE

        ++m_frame_count;

        FrameMark;
    }

    m_running = false;

    if (m_context->IsInitialized()) {
        m_context->GetCommandQueue()->WaitIdle();
    }
}

void sge::IEngine::Update() {
    const double current_tick = glfwGetTime();
    double delta_time = (current_tick - m_prev_tick);
    m_prev_tick = current_tick;

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

    Input::Clear();
}

void sge::IEngine::Render(const std::shared_ptr<sge::GlfwWindow>& window) {
    const LLGL::Extent2D size = window->GetContentSize();

    if (window->IsMinimized() || size.width <= 0 || size.height <= 0) {
        return;
    }

    OnRender(window);
    OnPostRender(window);

    if (m_auto_present) {
        GetRenderContext()->Present(*window);

        #if SGE_IMGUI_ENABLED
        ImGui::SetCurrentContext(m_context->GetOrCreateImGuiContext(*window));
        ImGuiIO& io = ImGui::GetIO();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            for (int i = 1; i < platform_io.Viewports.Size; i++) {
                ImGuiViewport* viewport = platform_io.Viewports[i];
                if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
                    continue;
                if (platform_io.Platform_SwapBuffers) platform_io.Platform_SwapBuffers(viewport, nullptr);
                if (platform_io.Renderer_SwapBuffers) platform_io.Renderer_SwapBuffers(viewport, nullptr);
            }
        }
        #endif
    }
}
