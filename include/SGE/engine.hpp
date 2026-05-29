#ifndef _SGE_ENGINE_HPP_
#define _SGE_ENGINE_HPP_

#pragma once

#include <expected>

#include <glm/vec2.hpp>

#include <SGE/input.hpp>
#include <SGE/window_manager.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/cursor_mode.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/utils/containers/swapbackvector.hpp>

#if SGE_IMGUI_ENABLED
    #include <backends/imgui_impl_glfw.h>
#endif

namespace sge {

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

class IEngine : GlfwWindow::EventListener {
protected:
    IEngine() = default;
    ~IEngine();
public:
    bool Init();
    void Run();
    void Stop() noexcept {
        m_running = false;
    }

    [[nodiscard]]
    const std::shared_ptr<RenderContext>& GetRenderContext() const noexcept {
        return m_context;
    }

    [[nodiscard]]
    uint64_t GetFrameCount() const noexcept {
        return m_frame_count;
    }

    [[nodiscard]]
    bool IsRunning() const noexcept {
        return m_running;
    }
protected: // Callbacks
    virtual bool OnInit() {
        return true;
    }
    virtual void OnPreUpdate() {}
    virtual void OnUpdate() {}
    virtual void OnPostUpdate() {}
    virtual void OnPreFixedUpdate() {}
    virtual void OnFixedUpdate() {}
    virtual void OnPostFixedUpdate() {}
    virtual void OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
        (void)window;
    }
    virtual void OnPostRender(const std::shared_ptr<sge::GlfwWindow>& window) {
        (void)window;
    }

    virtual void OnWindowResized(const std::shared_ptr<sge::GlfwWindow>& window, int width, int height) {
        (void)window; (void)width; (void)height;
    }
    virtual void OnFramebufferResize(const std::shared_ptr<sge::GlfwWindow>& window, int width, int height) {
        (void)window; (void)width; (void)height;
    }
    virtual void OnWindowDestroy(sge::GlfwWindow& window) {
        (void)window;
    }

protected:
    bool InitRenderContext(RenderBackend backend, ImGuiConfig imguiConfig = {}) {
        if (!m_context) {
            SGE_LOG_ERROR("Calling `InitRenderContext` before the engine is initialized. Did you forget to call `IEngine::Init` function?");
            return false;
        }

        return m_context->Init(backend, imguiConfig);
    }

    std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> CreateWindow(const WindowSettings& window_settings) {
        auto result = WindowManager::CreateWindow(window_settings);
        if (result.has_value()) {
            result.value()->Listen(*this);
        }
        return result;
    }

    void DestroyWindow(const std::shared_ptr<sge::GlfwWindow>& window) {
        OnWindowDestroy(*window);
        m_context->UnregisterWindow(*window);
        WindowManager::DestroyWindow(window);
    }

    inline void SetAutoPresent(bool auto_swap) noexcept {
        m_auto_present = auto_swap;
    }

protected:
    void OnWindowIconifyEvent(GLFWwindow* handle, bool iconified) final {
        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        window->m_flags.set(GlfwWindow::Flags::Minimized, iconified);
    }

    void OnWindowMaximizeEvent(GLFWwindow* handle, bool maximized) final {
        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        window->m_flags.set(GlfwWindow::Flags::Maximized, maximized);
    }

    void OnWindowFocusEvent(GLFWwindow* handle, bool focused) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_WindowFocusCallback(handle, focused);
    #endif

        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        window->m_flags.set(GlfwWindow::Flags::Focused, focused);
    }

    void OnWindowResizeEvent(GLFWwindow* handle, int width, int height) final {
        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        window->m_size.width = width;
        window->m_size.height = height;
        OnWindowResized(window, width, height);
    }

    void OnCursorPosEvent(GLFWwindow* handle, double xpos, double ypos) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CursorPosCallback(handle, xpos, ypos);
    #endif

        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        const glm::vec2 prev_pos = Input::CursorPosition();
        const glm::vec2 current_pos = glm::vec2(xpos, ypos);

        const glm::vec2 delta = current_pos - prev_pos;

        Input::SetMouseDelta(Input::MouseDelta() + delta);
        Input::SetCursorPosition(current_pos);
    }

    void OnFramebufferResizeEvent(GLFWwindow* handle, int width, int height) final {
        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);

        SGE_ASSERT(width >= 0);
        SGE_ASSERT(height >= 0);

        if (LLGL::SwapChain* swap_chain = m_context->GetSwapChain(*window)) {
            swap_chain->ResizeBuffers(LLGL::Extent2D(width, height));
        }
        
        OnFramebufferResize(window, width, height);
    }

    void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);


        if (ImGui::GetIO().WantCaptureKeyboard)
            return;
    #endif

        if (action == GLFW_PRESS) {
            Input::Press(static_cast<Key>(key), mods);
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<Key>(key), mods);
        }
    }

    void OnCharacterEvent(GLFWwindow* window, uint32_t codepoint) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CharCallback(window, codepoint);

        if (ImGui::GetIO().WantCaptureKeyboard)
            return;
    #endif
    
        Input::PushCodePoint(codepoint);
    }

    void OnMouseButtonEvent(GLFWwindow* window, const int button, const int action, const int mods) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())    
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        if (ImGui::GetIO().WantCaptureMouse)
            return;
    #endif
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<MouseButton>(button));
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<MouseButton>(button));
        }
    }

    void OnMouseScrollEvent(GLFWwindow* window, double xoffset, double yoffset) final {
    #if SGE_IMGUI_ENABLED
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

        if (ImGui::GetIO().WantCaptureMouse)
            return;
    #endif

        Input::PushMouseScrollEvent(yoffset);
    }

    void OnWindowRefreshEvent(GLFWwindow* handle) override {
        auto window = WindowManager::FindByHandle(handle);
        SGE_ASSERT(window != nullptr);
        
        Update();
        Render(window);
    }

private:
    void Render(const std::shared_ptr<sge::GlfwWindow>& window);
    void Update();
#if SGE_IMGUI_ENABLED
    void ForwardInputToImGui();
#endif

private:
    std::shared_ptr<RenderContext> m_context;
    double m_prev_tick = 0.0;
    uint64_t m_frame_count = 0;
    bool m_running = false;
    bool m_initialized = false;
    bool m_auto_present = true;
};

}

#endif