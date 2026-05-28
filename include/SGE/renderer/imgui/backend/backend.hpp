#ifndef SGE_RENDERER_IMGUI_BACKEND_HPP_
#define SGE_RENDERER_IMGUI_BACKEND_HPP_

#include <SGE/renderer/context.hpp>

namespace sge {

class ImGuiBackend {
public:
    ImGuiBackend(std::shared_ptr<sge::RenderContext> context) : m_context(std::move(context)) {

    }

    ImGuiContext* GetContext(const GlfwWindow& window);
    ImGuiContext* GetOrCreateContext(GlfwWindow& window);

    virtual void ReleaseContext(const GlfwWindow& window);
    virtual void BeginFrame(GlfwWindow& window);
    virtual void EndFrame() {}

protected:
    virtual void InitContext(ImGuiContext* context, GlfwWindow& window);

protected:
    std::unordered_map<uint32_t, ImGuiContext*> m_imgui_context_map;
    std::shared_ptr<sge::RenderContext> m_context;
};

}

#endif // SGE_RENDERER_IMGUI_BACKEND_HPP_