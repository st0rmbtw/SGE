#ifndef _SGE_RENDERER_CUSTOM_SURFACE_HPP_
#define _SGE_RENDERER_CUSTOM_SURFACE_HPP_

#pragma once

#include <GLFW/glfw3.h>
#include <LLGL/Surface.h>

#include <SGE/types/cursor_mode.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/log.hpp>
#include <glm/vec2.hpp>

namespace sge {

class GlfwWindow : public LLGL::Surface {
public:
    class EventListener {
    public:
        virtual void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
        virtual void OnCharacterEvent(GLFWwindow* window, uint32_t codepoint) = 0;
        virtual void OnCursorPosEvent(GLFWwindow* window, double xpos, double ypos) = 0;
        virtual void OnMouseButtonEvent(GLFWwindow* window, int button, int action, int mods) = 0;
        virtual void OnFramebufferResizeEvent(GLFWwindow* window, int width, int height) = 0;
        virtual void OnWindowIconifyEvent(GLFWwindow* window, bool iconified) = 0;
        virtual void OnWindowMaximizeEvent(GLFWwindow* window, bool maximized) = 0;
        virtual void OnMouseScrollEvent(GLFWwindow* window, double xoffset, double yoffset) = 0;
        virtual void OnWindowResizeEvent(GLFWwindow* window, int width, int height) = 0;
    };

    friend class IEngine;

public:
    GlfwWindow(GLFWwindow* wnd, LLGL::Extent2D size, glm::ivec2 position, uint8_t samples, bool fullscreen) :
        m_size(size),
        m_wnd(wnd),
        m_position(position),
        m_id(s_id++),
        m_samples(samples),
        m_fullscreen(fullscreen)
    {}

    GlfwWindow(GlfwWindow&& other) noexcept {
        other.m_wnd = nullptr;
        m_wnd = other.m_wnd;
        m_size = other.m_size;
        m_position = other.m_position;
        m_min_size = other.m_min_size;
        m_max_size = other.m_max_size;
        m_id = other.m_id;
        m_samples = other.m_samples;
        m_minimized = other.m_minimized;
        m_maximized = other.m_maximized;
        m_fullscreen = other.m_fullscreen;
    }

    ~GlfwWindow() override {
        if (m_wnd) {
            glfwDestroyWindow(m_wnd);
        }
    }

    bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;
    bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;

    void Listen(EventListener& listener) {
        glfwSetWindowUserPointer(m_wnd, &listener);

        glfwSetKeyCallback(m_wnd, HandleKeyboardEvents);
        glfwSetMouseButtonCallback(m_wnd, HandleMouseButtonEvents);
        glfwSetScrollCallback(m_wnd, HandleMouseScrollEvents);
        glfwSetCursorPosCallback(m_wnd, HandleCursorPosEvents);
        glfwSetWindowSizeCallback(m_wnd, HandleWindowResize);
        glfwSetWindowIconifyCallback(m_wnd, HandleWindowIconify);
        glfwSetWindowMaximizeCallback(m_wnd, HandleWindowMaximize);
        glfwSetFramebufferSizeCallback(m_wnd, HandleFramebufferResize);
        glfwSetCharCallback(m_wnd, HandleCharacterCallback);
    }

    [[nodiscard]]
    LLGL::Extent2D GetContentSize() const override {
        int width, height;
        glfwGetFramebufferSize(m_wnd, &width, &height);
        return LLGL::Extent2D(width, height);
    }

    [[nodiscard]]
    LLGL::Extent2D GetSize() const noexcept {
        return m_size;
    }

    [[nodiscard]]
    uint32_t GetWidth() const noexcept {
        return m_size.width;
    }

    [[nodiscard]]
    uint32_t GetHeight() const noexcept {
        return m_size.height;
    }

    [[nodiscard]]
    LLGL::Display* FindResidentDisplay() const override {
        return LLGL::Display::GetPrimary();
    }

    /*!
     *  @param[in] min_width The minimum width of the content
     *  area of the primary window, or -1 for any.
     *  @param[in] min_height The minimum height of the content
     *  area of the primary window, or -1 for any.
     */
    void SetWindowMinSize(int min_width, int min_height) {
        m_min_size.x = min_width;
        m_min_size.y = min_height;
        glfwSetWindowSizeLimits(m_wnd, min_width, min_height, m_max_size.x, m_max_size.y);
    }

    /*!
     *  @param[in] max_width The maximum width of the content
     *  area of the primary window, or -1 for any.
     *  @param[in] max_height The maximum height of the content
     *  area of the primary window, or -1 for any.
     */
    void SetWindowMaxSize(int max_width, int max_height) {
        m_max_size.x = max_width;
        m_max_size.y = max_height;
        glfwSetWindowSizeLimits(m_wnd, m_min_size.x, m_min_size.y, max_width, max_height);
    }

    void ShowWindow() {
        glfwShowWindow(m_wnd);
    }

    void HideWindow() {
        glfwHideWindow(m_wnd);
    }

    void SetCursorMode(CursorMode cursor_mode) {
        glfwSetInputMode(m_wnd, GLFW_CURSOR, static_cast<int>(cursor_mode));
    }

    void SetMaximized() {
        glfwMaximizeWindow(m_wnd);
        m_maximized = true;
    }

    void SetMinimized() {
        glfwIconifyWindow(m_wnd);
        m_minimized = true;
    }

    void SetFullscreen() {
        glfwGetWindowPos(m_wnd, &m_position.x, &m_position.y);
        glfwSetWindowMonitor(m_wnd, glfwGetPrimaryMonitor(), 0, 0, m_size.width, m_size.height, GLFW_DONT_CARE);
    }

    void SetWindowed() {
        glfwSetWindowMonitor(m_wnd, nullptr, m_position.x, m_position.y, m_size.width, m_size.height, GLFW_DONT_CARE);
    }

    void Restore() {
        glfwRestoreWindow(m_wnd);
        m_minimized = false;
        m_maximized = false;
    }

    [[nodiscard]]
    bool IsMinimized() const noexcept {
        return m_minimized;
    }

    [[nodiscard]]
    bool IsMaximized() const noexcept {
        return m_maximized;
    }

    [[nodiscard]]
    bool IsFullscreen() const noexcept {
        return m_fullscreen;
    }

    [[nodiscard]]
    uint8_t GetSamples() const noexcept {
        return m_samples;
    }

    [[nodiscard]]
    uint32_t GetID() const noexcept {
        return m_id;
    }

    [[nodiscard]]
    glm::ivec2 GetPosition() const noexcept {
        return m_position;
    }

    [[nodiscard]]
    bool ShouldBeClosed() const noexcept {
        return glfwWindowShouldClose(m_wnd);
    }
private:
    static void HandleKeyboardEvents(GLFWwindow* window, int key, int scancode, int action, int mods) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnKeyEvent(window, key, scancode, action, mods);
    }
    static void HandleMouseButtonEvents(GLFWwindow* window, int button, int action, int mods) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnMouseButtonEvent(window, button, action, mods);
    }
    static void HandleMouseScrollEvents(GLFWwindow* window, double xoffset, double yoffset) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnMouseScrollEvent(window, xoffset, yoffset);
    }
    static void HandleCursorPosEvents(GLFWwindow* window, double xpos, double ypos) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnCursorPosEvent(window, xpos, ypos);
    }
    static void HandleCharacterCallback(GLFWwindow* window, uint32_t codepoint) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnCharacterEvent(window, codepoint);
    }
    static void HandleWindowResize(GLFWwindow* window, int width, int height) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnWindowResizeEvent(window, width, height);
    }
    static void HandleFramebufferResize(GLFWwindow* window, int width, int height) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnFramebufferResizeEvent(window, width, height);
    }
    static void HandleWindowIconify(GLFWwindow* window, int iconified) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnWindowIconifyEvent(window, iconified == GLFW_TRUE);
    }
    static void HandleWindowMaximize(GLFWwindow* window, int maximized) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnWindowMaximizeEvent(window, maximized == GLFW_TRUE);
    }

private:
    LLGL::Extent2D m_size;
    GLFWwindow* m_wnd = nullptr;
    glm::ivec2 m_min_size = glm::ivec2(0, 0);
    glm::ivec2 m_max_size = glm::ivec2(INT_MAX, INT_MAX);
    glm::ivec2 m_position = glm::ivec2(0, 0);
    uint32_t m_id = 0;
    uint8_t m_samples = 1;
    bool m_minimized = false;
    bool m_maximized = false;
    bool m_fullscreen = false;

    static std::atomic<uint32_t> s_id;
};

}

#endif