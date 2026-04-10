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
#include <utility>

namespace sge {

inline constexpr const char* DEFAULT_CACHE_DIR = "./cache/pipeline/";

class IEngine : GlfwWindow::EventListener {
    using WindowMap = std::unordered_map<GLFWwindow*, std::shared_ptr<sge::GlfwWindow>>;
    using WindowFlags = GlfwWindow::Flags;

protected:
    class WindowIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = GlfwWindow;
        using difference_type = typename WindowMap::difference_type;
        using pointer = value_type*;
        using reference = value_type&;

        explicit WindowIterator(WindowMap::iterator it) : current(std::move(it)) {   
        }

        reference operator*() const { return *current->second.get(); }
        pointer operator->() const { return current->second.get(); }

        WindowIterator& operator++() {
            ++current;
            return *this;
        }

        WindowIterator operator++(int) {
            WindowIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const WindowIterator& other) const { return current == other.current; }
        bool operator!=(const WindowIterator& other) const { return current != other.current; }

    private:
        WindowMap::iterator current;
    };

    class WindowContainer {
    public:
        WindowContainer(WindowMap::iterator begin, WindowMap::iterator end) :
            m_begin(std::move(begin)), m_end(std::move(end)) {}

        WindowIterator begin() { return WindowIterator(m_begin); }
        WindowIterator end() { return WindowIterator(m_end); }
    private:
        WindowMap::iterator m_begin;
        WindowMap::iterator m_end;
    };

protected:
    IEngine() = default;
    ~IEngine();
public:
    virtual bool Init();

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
    bool InitRenderContext(RenderBackend backend) {
        if (!m_context) {
            SGE_LOG_ERROR("Calling `InitRenderContext` before the engine is initialized. Did you forget to call `IEngine::Init` function?");
            return false;
        }

        return m_context->Init(backend);
    }

    std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> CreateWindow(const WindowSettings& window_settings);

    void DestroyWindow(const std::shared_ptr<sge::GlfwWindow>& window) {
        OnWindowDestroy(*window);
        m_context->UnregisterWindow(*window);
        m_window_map.erase(window->m_wnd);
    }

    WindowContainer IterWindows() {
        return WindowContainer(m_window_map.begin(), m_window_map.end());
    }

    sge::GlfwWindow* GetFocusedWindow() {
        for (const auto& [ptr, window] : m_window_map) {
            if (window->IsFocused()) return window.get();
        }
        return nullptr;
    }

protected:
    void OnWindowIconifyEvent(GLFWwindow* window, bool iconified) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        it->second->m_flags.set(WindowFlags::Minimized, iconified);
    }

    void OnWindowMaximizeEvent(GLFWwindow* window, bool maximized) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        it->second->m_flags.set(WindowFlags::Maximized, maximized);
    }

    void OnWindowFocusEvent(GLFWwindow *window, bool focused) final {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        it->second->m_flags.set(WindowFlags::Focused, focused);
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

        const glm::vec2 prev_pos = Input::CursorPosition();
        const glm::vec2 current_pos = glm::vec2(xpos, ypos);

        const glm::vec2 delta = current_pos - prev_pos;

        Input::SetMouseDelta(Input::MouseDelta() + delta);
        Input::SetCursorPosition(current_pos);
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

    void OnWindowRefreshEvent(GLFWwindow *window) override {
        auto it = m_window_map.find(window);
        SGE_ASSERT(it != m_window_map.end());
        OnRender(it->second);
        OnPostRender(it->second);
    }
private:
    std::shared_ptr<RenderContext> m_context;
    WindowMap m_window_map;
    uint64_t m_frame_count = 0;
    bool m_running = false;
    bool m_initialized = false;
};

}

#endif