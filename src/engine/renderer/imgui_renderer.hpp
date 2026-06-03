#ifndef ENGINE_RENDERER_IMGUI_RENDERER_HPP_
#define ENGINE_RENDERER_IMGUI_RENDERER_HPP_

#include <SGE/renderer/context.hpp>
#include <imgui.h>

namespace ImGuiRenderer {
    bool Init(std::shared_ptr<sge::RenderContext> context);
    void Shutdown();
    void NewFrame();
    void RenderDrawData(ImDrawData* data);
} // namespace ImGuiRenderer

#endif // ENGINE_RENDERER_IMGUI_RENDERER_HPP_