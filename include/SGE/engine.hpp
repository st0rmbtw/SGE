#ifndef _SGE_ENGINE_HPP_
#define _SGE_ENGINE_HPP_

#pragma once

#include <expected>
#include <unordered_map>

#include <glm/vec2.hpp>

#include <SGE/input.hpp>
#include <SGE/renderer/glfw_surface.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/cursor_mode.hpp>
#include <SGE/renderer/renderer.hpp>

namespace sge {

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

struct EngineConfig {
    sge::WindowSettings window_settings;
    const char* pipeline_cache_path = DEFAULT_CACHE_DIR;
    bool cache_pipelines = true;
};

using WindowId = uint32_t;

class IEngine : GlfwWindow::EventListener {
protected:
    IEngine();

public:
    void Run();

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
    virtual void OnRender() {}
    virtual void OnPostRender() {}

    virtual void OnWindowResized(GlfwWindow& window, int width, int height) {};
    virtual void OnFramebufferResize(GlfwWindow& window, int width, int height) {};

protected:

    std::expected<std::shared_ptr<GlfwWindow>, const char*> CreateWindow(const WindowSettings& window_settings) {
        glfwWindowHint(GLFW_FOCUSED, 1);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, window_settings.samples);
        glfwWindowHint(GLFW_RESIZABLE, window_settings.resizable ? GLFW_TRUE : GLFW_FALSE);

        GLFWmonitor* primary_monitor = window_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        GLFWwindow *window = glfwCreateWindow(window_settings.width, window_settings.height, window_settings.title, primary_monitor, nullptr);
        if (window == nullptr) {
            const char* description = nullptr;
            glfwGetError(&description);
            return std::unexpected(description);
        }

        glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(window_settings.cursor_mode));

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        std::shared_ptr<GlfwWindow> instance = std::make_shared<GlfwWindow>(window, LLGL::Extent2D(width, height));
        m_windows.push_back(instance);
        m_window_map.try_emplace(window, instance.get());
        return instance;
    }

    void DestroyWindow(const std::shared_ptr<GlfwWindow>& window) {
        m_window_map.erase(window->m_wnd);
        std::find(m_windows.begin(), m_windows.end(), window);
    }

protected:
    void OnWindowIconifyEvent(GLFWwindow *window, int iconified) override {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_minimized = iconified == GLFW_TRUE;
    }

    void OnWindowResizeEvent(GLFWwindow *window, int width, int height) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        it->second->m_size.width = width;
        it->second->m_size.height = height;
        OnWindowResized(*it->second, width, height);
    }

    void OnCursorPosEvent(GLFWwindow *window, double xpos, double ypos) override {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());

        const LLGL::Extent2D window_size = it->second->GetSize();

        xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.width));
        ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.height));

        Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
    }

    void OnFramebufferResizeEvent(GLFWwindow *window, int width, int height) override {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        
        OnFramebufferResize(*it->second, width, height);
    }

    void OnKeyEvent(GLFWwindow*, int key, int scancode, int action, int mods) override {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<Key>(key), mods);
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<Key>(key), mods);
        }
    }

    void OnMouseButtonEvent(GLFWwindow*, int button, int action, int mods) override {
        if (action == GLFW_PRESS) {
            Input::Press(static_cast<MouseButton>(button));
        } else if (action == GLFW_RELEASE) {
            Input::Release(static_cast<MouseButton>(button));
        }
    }

    void OnMouseScrollEvent(GLFWwindow *window, double xoffset, double yoffset) override {
        Input::PushMouseScrollEvent(yoffset);
    }

    virtual bool ShouldStop() {
        return false;
    }
private:
    std::unordered_map<GLFWwindow*, GlfwWindow*> m_window_map;
    std::vector<std::shared_ptr<GlfwWindow>> m_windows;
    uint32_t m_frame_count = 0;
    bool m_running = false;
};

namespace Engine {
    using PreUpdateCallback = void (*)(void);
    using UpdateCallback = void (*)(void);
    using PostUpdateCallback = void (*)(void);
    using FixedUpdateCallback = void (*)(void);
    using RenderCallback = void (*)(void);
    using PostRenderCallback = void (*)(void);
    using CleanupCallback = void (*)(void);
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
    void SetCleanupCallback(CleanupCallback);
    void SetWindowResizeCallback(WindowResizeCallback);
    void SetLoadAssetsCallback(LoadAssetsCallback);

    uint32_t GetFrameCount();

    void Run();
    void Stop();
    void Destroy();

    sge::Renderer& Renderer();
};

}

#endif