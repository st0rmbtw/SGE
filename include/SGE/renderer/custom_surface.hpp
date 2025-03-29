#ifndef _SGE_RENDERER_CUSTOM_SURFACE_HPP_
#define _SGE_RENDERER_CUSTOM_SURFACE_HPP_

#pragma once

#include <GLFW/glfw3.h>
#include <LLGL/Surface.h>

#include "../defines.hpp"

_SGE_BEGIN

namespace renderer {

#if defined(SGE_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SGE_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#else
    #if defined(WAYLAND)
        #define GLFW_EXPOSE_NATIVE_WAYLAND
    #elif defined(X11)
        #define GLFW_EXPOSE_NATIVE_X11
  #endif
#endif

class CustomSurface : public LLGL::Surface {
public:
    CustomSurface(GLFWwindow* window, const LLGL::Extent2D& size);
    CustomSurface(CustomSurface&& other) noexcept;
    ~CustomSurface() override;

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

_SGE_END

#endif