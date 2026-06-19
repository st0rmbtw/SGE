#ifndef _SGE_RENDERER_HPP_
#define _SGE_RENDERER_HPP_

#include <GLFW/glfw3.h>

#include <LLGL/CommandBufferFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/Shader.h>
#include <LLGL/Utils/Utility.h>

#include <SGE/defines.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/macros.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/backend.hpp>
#include <SGE/types/shader_def.hpp>
#include <SGE/types/shader_path.hpp>
#include <SGE/types/texture.hpp>
#include <SGE/types/window_settings.hpp>

#include <memory>

namespace sge {

struct SGE_ALIGN(16) GlobalUniforms {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 inv_view_proj_matrix;
    glm::mat4 nonscale_view_projection_matrix;
    glm::mat4 nonscale_projection_matrix;
    glm::vec2 camera_position;
    glm::uvec2 window_size;
};

class Renderer {
public:
    explicit Renderer(const std::shared_ptr<RenderContext>& context);

    inline void Begin() {
        m_command_buffer->Begin();
    }

    inline void End() {
        m_command_buffer->End();
        m_command_queue->Submit(*m_command_buffer);
    }

    void BeginPass(LLGL::RenderTarget& target, const Camera& camera);

    inline void BeginPass(const std::shared_ptr<GlfwWindow>& window, const Camera& camera) {
        BeginPass(m_context->GetOrCreateSwapChain(window), camera);
    }

    inline void Clear(const LLGL::ClearValue& clear_value = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), long clear_flags = LLGL::ClearFlags::Color) {
        m_command_buffer->Clear(clear_flags, clear_value);
    }

    inline void EndPass() {
        m_context->PopRenderTarget();
        m_command_buffer->EndRenderPass();
    }

    inline void SetScissor(const LLGL::Scissor& scissor) {
        m_command_buffer->SetScissor(scissor);
    }

    void Present(const std::shared_ptr<GlfwWindow>& window) {
        m_context->Present(*window);
    }

    [[nodiscard]]
    inline LLGL::CommandBuffer* CommandBuffer() const noexcept {
        return m_command_buffer;
    }

    [[nodiscard]]
    inline LLGL::CommandQueue* CommandQueue() const noexcept {
        return m_command_queue;
    }

    [[nodiscard]]
    inline const Unique<LLGL::Buffer>& GlobalUniformBuffer() const noexcept {
        return m_uniform_buffer;
    }

    [[nodiscard]]
    inline const std::shared_ptr<RenderContext>& GetRenderContext() const noexcept {
        return m_context;
    }

protected:
    Unique<LLGL::Buffer> m_uniform_buffer;
    
    std::shared_ptr<RenderContext> m_context;
    
    LLGL::CommandQueue* m_command_queue = nullptr;
    LLGL::CommandBuffer* m_command_buffer = nullptr;

    LLGL::Extent2D m_viewport = LLGL::Extent2D(0, 0);

};

} // namespace sge

#endif
