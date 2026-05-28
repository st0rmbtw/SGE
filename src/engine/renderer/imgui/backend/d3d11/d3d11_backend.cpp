#include <LLGL/Backend/Direct3D11/NativeHandle.h>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>

#include "d3d11_backend.hpp"

void sge::ImGuiBackendDirect3D11::InitContext(ImGuiContext* context, GlfwWindow& window) {
    ImGuiBackend::InitContext(context, window);

    LLGL::Direct3D11::RenderSystemNativeHandle nativeDeviceHandle;
    m_context->GetLLGLContext()->GetNativeHandle(&nativeDeviceHandle, sizeof(nativeDeviceHandle));
    m_d3d_device = nativeDeviceHandle.device;

    LLGL::Direct3D11::CommandBufferNativeHandle nativeContextHandle;
    m_context->GetCommandBuffer()->GetNativeHandle(&nativeContextHandle, sizeof(nativeContextHandle));
    m_d3d_device_context = nativeContextHandle.deviceContext;

    ImGui_ImplDX11_Init(m_d3d_device, m_d3d_device_context);
}

sge::ImGuiBackendDirect3D11::~ImGuiBackendDirect3D11() {
    if (m_d3d_device != nullptr)
        m_d3d_device->Release();
    if (m_d3d_device_context != nullptr)
        m_d3d_device_context->Release();
}

void sge::ImGuiBackendDirect3D11::ReleaseContext(const GlfwWindow& window) {
    ImGui::SetCurrentContext(GetContext(window));

    ImGui_ImplDX11_Shutdown();

    ImGuiBackend::ReleaseContext(window);
}

void sge::ImGuiBackendDirect3D11::BeginFrame(GlfwWindow& window) {
    ImGuiBackend::BeginFrame(window);
    ImGui_ImplDX11_NewFrame();
}

void sge::ImGuiBackendDirect3D11::EndFrame() {
    ImGuiBackend::EndFrame();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}