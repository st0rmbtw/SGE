#ifndef SGE_RENDERER_IMGUI_BACKEND_D3D11_D3D11_BACKEND_HPP_
#define SGE_RENDERER_IMGUI_BACKEND_D3D11_D3D11_BACKEND_HPP_

#include <SGE/renderer/context.hpp>
#include <SGE/renderer/imgui/backend/backend.hpp>
#include <d3d11.h>

namespace sge {

class ImGuiBackendDirect3D11 : public ImGuiBackend {
public:
    ImGuiBackendDirect3D11(std::shared_ptr<sge::RenderContext> context) : ImGuiBackend(std::move(context)) {}
    ~ImGuiBackendDirect3D11();

    void InitContext(ImGuiContext *context, GlfwWindow &window) override;
    void ReleaseContext(const GlfwWindow &window) override;
    void BeginFrame(GlfwWindow &window) override;
    void EndFrame() override;
private:
    ID3D11Device* m_d3d_device = nullptr;
    ID3D11DeviceContext* m_d3d_device_context = nullptr;
};

}

#endif // SGE_RENDERER_IMGUI_BACKEND_D3D11_D3D11_BACKEND_HPP_