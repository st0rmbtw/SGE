#include <LLGL/Platform/NativeHandle.h>
#include <SGE/renderer/imgui/backend/backend.hpp>

#include <SGE/defines.hpp>

#include <backends/imgui_impl_glfw.h>

#if SGE_PLATFORM_APPLE
static NSView* GetContentViewFromNativeHandle(const LLGL::NativeHandle& nativeHandle)
{
    if ([nativeHandle.responder isKindOfClass:[NSWindow class]])
    {
        /* Interpret responder as NSWindow */
        return [(NSWindow*)nativeHandle.responder contentView];
    }
    if ([nativeHandle.responder isKindOfClass:[NSView class]])
    {
        /* Interpret responder as NSView */
        return (NSView*)nativeHandle.responder;
    }
    LLGL::Trap(LLGL::Exception::InvalidArgument, __FUNCTION__, "NativeHandle::responder is neither of type NSWindow nor NSView");
}

static NSView* GetNSViewFromSurface(LLGL::Surface& surface)
{
    LLGL::NativeHandle nativeHandle;
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    return GetContentViewFromNativeHandle(nativeHandle);
}
#endif

static void PlatformInit(sge::GlfwWindow& window) {
    LLGL::NativeHandle nativeHandle;
    window.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    ImGui_ImplGlfw_InitForOther(window.GetGlfwHandle(), false);
}

static void PlatformShutdown() {
    ImGui_ImplGlfw_Shutdown();
}

static void PlatformNewFrame() {
    ImGui_ImplGlfw_NewFrame();
}

ImGuiContext* sge::ImGuiBackend::GetContext(const GlfwWindow& window) {
    auto it = m_imgui_context_map.find(window.GetID());
    if (it == m_imgui_context_map.end()) {
        return nullptr;
    }
    return it->second;
}

ImGuiContext* sge::ImGuiBackend::GetOrCreateContext(GlfwWindow& window) {
    ImGuiContext* context = nullptr;

    auto it = m_imgui_context_map.find(window.GetID());
    if (it == m_imgui_context_map.end()) {
        context = ImGui::CreateContext();
        {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
        InitContext(context, window);
        m_imgui_context_map.try_emplace(window.GetID(), context);
    } else {
        context = it->second;
    }
    
    return context;
}

void sge::ImGuiBackend::InitContext(ImGuiContext* context, GlfwWindow& window) {
    ImGui::SetCurrentContext(context);
    ImGui::StyleColorsDark();
    PlatformInit(window);
}

void sge::ImGuiBackend::ReleaseContext(const GlfwWindow& window) {
    ImGuiContext* context = GetContext(window);

    ImGui::SetCurrentContext(context);

    PlatformShutdown();

    ImGui::DestroyContext(context);
}

void sge::ImGuiBackend::BeginFrame(sge::GlfwWindow& window) {
    ImGui::SetCurrentContext(GetOrCreateContext(window));
    PlatformNewFrame();
}