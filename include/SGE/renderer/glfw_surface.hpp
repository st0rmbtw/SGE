#ifndef _SGE_RENDERER_CUSTOM_SURFACE_HPP_
#define _SGE_RENDERER_CUSTOM_SURFACE_HPP_

#pragma once

#include <GLFW/glfw3.h>
#include <LLGL/Surface.h>

#include "../defines.hpp"

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

class GlfwSurface : public LLGL::Surface {
public:
    GlfwSurface(GLFWwindow* window, const LLGL::Extent2D& size);
    GlfwSurface(GlfwSurface&& other) noexcept;
    ~GlfwSurface() override;

    bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;
    bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;

    [[nodiscard]] LLGL::Extent2D GetContentSize() const override;
    [[nodiscard]] LLGL::Display* FindResidentDisplay() const override;

    bool ProcessEvents();
private:
    LLGL::Extent2D m_size;
    GLFWwindow* m_wnd = nullptr;
};

}

#endif