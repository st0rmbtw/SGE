#ifndef SGE_RENDERER_IMGUI_BACKEND_METAL_METAL_BACKEND_HPP_
#define SGE_RENDERER_IMGUI_BACKEND_METAL_METAL_BACKEND_HPP_

#include <SGE/renderer/imgui/backend/backend.hpp>

namespace sge {

class ImGuiBackendMetal : public ImGuiBackend {
public:
    ImGuiBackendMetal(std::shared_ptr<sge::RenderContext> context) : ImGuiBackend(std::move(context)) {}

    void InitContext(ImGuiContext *context, GlfwWindow &window) override;
    void ReleaseContext(const GlfwWindow &window) override;
    void BeginFrame(GlfwWindow &window) override;
    void EndFrame() override;
};

}

#endif // SGE_RENDERER_IMGUI_BACKEND_METAL_METAL_BACKEND_HPP_