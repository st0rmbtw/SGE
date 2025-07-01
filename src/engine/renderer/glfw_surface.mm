#include <SGE/renderer/glfw_surface.hpp>

#include <LLGL/Platform/NativeHandle.h>
#include <GLFW/glfw3native.h>

#include <SGE/defines.hpp>

using namespace sge;

GlfwSurface::GlfwSurface(GLFWwindow* window, const LLGL::Extent2D& size) :
    m_size(size),
    m_wnd(window) {}

GlfwSurface::GlfwSurface(GlfwSurface&& other) noexcept :
    m_size(other.m_size),
    m_wnd(other.m_wnd)
{
    other.m_wnd = nullptr;
}

GlfwSurface::~GlfwSurface() {
    if (m_wnd) glfwDestroyWindow(m_wnd);
}

bool GlfwSurface::GetNativeHandle(void* nativeHandle, std::size_t) {
    auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
#if defined(SGE_PLATFORM_WINDOWS)
    handle->window = glfwGetWin32Window(m_wnd);
#elif defined(SGE_PLATFORM_MACOS)
    handle->responder = glfwGetCocoaWindow(m_wnd);
#elif defined(SGE_PLATFORM_LINUX)
    int platform = glfwGetPlatform();

    if (platform == GLFW_PLATFORM_WAYLAND) {
        handle->wayland.window = glfwGetWaylandWindow(m_wnd);
        handle->wayland.display = glfwGetWaylandDisplay();
        handle->type = LLGL::NativeType::Wayland;
    } else {
        handle->window = glfwGetX11Window(m_wnd);
        handle->display = glfwGetX11Display();
        handle->type = LLGL::NativeType::X11;
    }

#endif
    return true;
}

LLGL::Extent2D GlfwSurface::GetContentSize() const {
    return m_size;
}

bool GlfwSurface::AdaptForVideoMode(LLGL::Extent2D* resolution, bool*) {
    m_size = *resolution;
    glfwSetWindowSize(m_wnd, m_size.width, m_size.height);
    return true;
}

LLGL::Display* GlfwSurface::FindResidentDisplay() const {
    return LLGL::Display::GetPrimary();
}

bool GlfwSurface::ProcessEvents() {
    glfwPollEvents();
    return !glfwWindowShouldClose(m_wnd);
}