#ifndef _SGE_RENDERER_CUSTOM_SURFACE_HPP_
#define _SGE_RENDERER_CUSTOM_SURFACE_HPP_

#pragma once

#include <GLFW/glfw3.h>
#include <LLGL/Surface.h>

#include <SGE/types/cursor_mode.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/log.hpp>
#include <glm/vec2.hpp>
#include "../defines.hpp"

#include <expected>

namespace sge {

#if SGE_PLATFORM_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif SGE_PLATFORM_MACOS
    #define GLFW_EXPOSE_NATIVE_COCOA
#else
    #if SGE_PLATFORM_LINUX
        #define GLFW_EXPOSE_NATIVE_WAYLAND
        #define GLFW_EXPOSE_NATIVE_X11
    #endif
#endif

class GlfwWindow : public LLGL::Surface {
public:
    class EventListener {
    public:
        virtual void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {};
        virtual void OnCursorPosEvent(GLFWwindow* window, double xpos, double ypos) {};
        virtual void OnMouseButtonEvent(GLFWwindow* window, int button, int action, int mods) {};
        virtual void OnFramebufferResizeEvent(GLFWwindow* window, int width, int height) {};
        virtual void OnWindowIconifyEvent(GLFWwindow* window, int iconified) {};
        virtual void OnMouseScrollEvent(GLFWwindow* window, double xoffset, double yoffset) {};
        virtual void OnWindowResizeEvent(GLFWwindow* window, int width, int height) {};
    };

    friend class IEngine;

public:
    GlfwWindow(GLFWwindow* wnd, LLGL::Extent2D size) : m_size(size), m_wnd(wnd) {}

    GlfwWindow(GlfwWindow&& other) noexcept : m_size(other.m_size), m_wnd(other.m_wnd) {
        other.m_wnd = nullptr;
    }
    ~GlfwWindow() override {
        if (m_wnd) glfwDestroyWindow(m_wnd);
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
    LLGL::Extent2D GetSize() const {
        return m_size;
    }

    [[nodiscard]]
    LLGL::Display* FindResidentDisplay() const override {
        return LLGL::Display::GetPrimary();
    }

    bool ProcessEvents() {
        glfwPollEvents();
        return !glfwWindowShouldClose(m_wnd);
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

    [[nodiscard]]
    bool IsMinimized() const {
        return m_minimized;
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
    }
    static void HandleWindowResize(GLFWwindow* window, int width, int height) {
        EventListener* listener = static_cast<EventListener*>(glfwGetWindowUserPointer(window));
        if (listener == nullptr)
            return;
        listener->OnWindowResizeEvent(window, width, height);

        // if (width <= 0 || height <= 0) {
        //     wnd->m_size.width = 0;
        //     wnd->m_size.height = 0;
        //     return;
        // }        

        // wnd->m_size.width = width;
        // wnd->m_size.height = height;
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
        // wnd->m_minimized = iconified == GLFW_TRUE;
    }

private:
    LLGL::Extent2D m_size;
    GLFWwindow* m_wnd = nullptr;
    glm::ivec2 m_min_size = glm::ivec2(0, 0);
    glm::ivec2 m_max_size = glm::ivec2(INT_MAX, INT_MAX);
    bool m_minimized = false;
};

}

#endif