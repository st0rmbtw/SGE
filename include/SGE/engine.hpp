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
#include <SGE/renderer/renderer.hpp>
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
    virtual void OnRender(const std::shared_ptr<GlfwWindow>& window) {}
    virtual void OnPostRender(const std::shared_ptr<GlfwWindow>& window) {}

    virtual void OnWindowResized(const std::shared_ptr<GlfwWindow>& window, int width, int height) {}
    virtual void OnFramebufferResize(const std::shared_ptr<GlfwWindow>& window, int width, int height) {}
    virtual void OnWindowDestroy(GlfwWindow& window) {}

protected:

    std::expected<GlfwWindow*, const char*> CreateWindow(const WindowSettings& window_settings);
    void DestroyWindow(const std::shared_ptr<GlfwWindow>& window) {
        OnWindowDestroy(*window);
        m_window_map.erase(window->m_wnd);
    }

protected:
    void OnWindowIconifyEvent(GLFWwindow *window, int iconified) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_minimized = iconified == GLFW_TRUE;
    }

    void OnWindowResizeEvent(GLFWwindow *window, int width, int height) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_size.width = width;
        it->second->m_size.height = height;
        OnWindowResized(it->second, width, height);
    }

    void OnCursorPosEvent(GLFWwindow *window, double xpos, double ypos) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        const LLGL::Extent2D window_size = it->second->GetSize();

        xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.width));
        ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.height));

        Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
    }

    void OnFramebufferResizeEvent(GLFWwindow *window, int width, int height) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        
        OnFramebufferResize(it->second, width, height);
    }

    void OnKeyEvent(GLFWwindow*, int key, int scancode, int action, int mods) final {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<Key>(key), mods);
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<Key>(key), mods);
        }
    }

    void OnMouseButtonEvent(GLFWwindow*, int button, int action, int mods) final {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<MouseButton>(button));
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<MouseButton>(button));
        }
    }

    void OnMouseScrollEvent(GLFWwindow *window, double xoffset, double yoffset) final {
        Input::PushMouseScrollEvent(yoffset);
    }
private:
    class Renderer m_renderer;
    std::unordered_map<GLFWwindow*, std::shared_ptr<GlfwWindow>> m_window_map;
    uint64_t m_frame_count = 0;
    bool m_running = false;
};

}

#endif