#ifndef _SGE_ENGINE_HPP_
#define _SGE_ENGINE_HPP_

#pragma once

#include <expected>
#include <unordered_map>

#include <glm/vec2.hpp>

#include <SGE/input.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/cursor_mode.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/utils/containers/swapbackvector.hpp>

namespace sge {

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

using WindowId = uint32_t;

class IEngine : GlfwWindow::EventListener {
protected:
    IEngine();
    ~IEngine();
public:
    void Run();
    void Stop() {
        m_running = false;
    }

    [[nodiscard]]
    const std::shared_ptr<RenderContext>& GetRenderContext() const {
        return m_context;
    }

    [[nodiscard]]
    uint64_t GetFrameCount() const {
        return m_frame_count;
    }

    [[nodiscard]]
    bool IsRunning() const {
        return m_running;
    }
protected: // Callbacks
    virtual void OnPreUpdate() {}
    virtual void OnUpdate() {}
    virtual void OnPostUpdate() {}
    virtual void OnFixedUpdate() {}
    virtual void OnFixedPostUpdate() {}
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
    bool InitRenderContext(RenderBackend backend) {
        return m_context->Init(backend);
    }

    std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> CreateWindow(const WindowSettings& window_settings);

    void DestroyWindow(const std::shared_ptr<sge::GlfwWindow>& window) {
        OnWindowDestroy(*window);
        m_context->UnregisterWindow(*window);
        m_window_map.erase(window->m_wnd);
    }

protected:
    void OnWindowIconifyEvent(GLFWwindow* window, bool iconified) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_minimized = iconified;
    }

    void OnWindowMaximizeEvent(GLFWwindow* window, bool maximized) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_maximized = maximized;
    }

    void OnWindowResizeEvent(GLFWwindow* window, int width, int height) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_size.width = width;
        it->second->m_size.height = height;
        OnWindowResized(it->second, width, height);
    }

    void OnCursorPosEvent(GLFWwindow* window, double xpos, double ypos) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        const LLGL::Extent2D window_size = it->second->GetSize();

        xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.width));
        ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.height));

        Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
    }

    void OnFramebufferResizeEvent(GLFWwindow* window, int width, int height) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        SGE_ASSERT(width >= 0);
        SGE_ASSERT(height >= 0);

        if (LLGL::SwapChain* swap_chain = m_context->GetSwapChain(it->second)) {
            swap_chain->ResizeBuffers(LLGL::Extent2D(width, height));
        }
        
        OnFramebufferResize(it->second, width, height);
    }

    void OnKeyEvent(GLFWwindow*, int key, int /* scancode */, int action, int mods) final {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<Key>(key), mods);
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<Key>(key), mods);
        }
    }

    void OnCharacterEvent(GLFWwindow*, uint32_t codepoint) final {
        Input::PushCodePoint(codepoint);
    }

    void OnMouseButtonEvent(GLFWwindow*, int button, int action, int /* mods */) final {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<MouseButton>(button));
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<MouseButton>(button));
        }
    }

    void OnMouseScrollEvent(GLFWwindow*, double /* xoffset */, double yoffset) final {
        Input::PushMouseScrollEvent(yoffset);
    }
private:
    std::shared_ptr<RenderContext> m_context;
    std::unordered_map<GLFWwindow*, std::shared_ptr<GlfwWindow>> m_window_map;
    uint64_t m_frame_count = 0;
    bool m_running = false;
};

}

#endif