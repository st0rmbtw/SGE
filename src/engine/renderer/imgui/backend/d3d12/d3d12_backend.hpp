#ifndef SGE_RENDERER_IMGUI_BACKEND_D3D12_D3D12_BACKEND_HPP_
#define SGE_RENDERER_IMGUI_BACKEND_D3D12_D3D12_BACKEND_HPP_

#include <SGE/renderer/imgui/backend/backend.hpp>

namespace sge {

class ImGuiBackendDirect3D12 : public ImGuiBackend {
public:
    ImGuiBackendDirect3D12(std::shared_ptr<sge::RenderContext> context) : ImGuiBackend(std::move(context)) {}

    void InitContext(ImGuiContext *context, GlfwWindow &window) override;
    void ReleaseContext(const GlfwWindow &window) override;
    void BeginFrame(GlfwWindow &window) override;
    void EndFrame() override;
};

}

#endif // SGE_RENDERER_IMGUI_BACKEND_D3D12_D3D12_BACKEND_HPP_