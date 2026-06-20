#ifndef _SGE_TYPES_FRAMEBUFFER_HPP_
#define _SGE_TYPES_FRAMEBUFFER_HPP_

#include <array>

#include <SGE/renderer/resource.hpp>

#include <LLGL/RenderPass.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTarget.h>
#include <LLGL/Texture.h>

namespace sge {

class Framebuffer {
    friend class Renderer;

public:
    Framebuffer() = default;

    explicit Framebuffer(sge::Ref<LLGL::Texture> texture, sge::Ref<LLGL::RenderTarget> target, sge::Ref<LLGL::RenderPass> renderPass = nullptr) :
        m_textures{ std::move(texture) },
        m_target(std::move(target)),
        m_render_pass(std::move(renderPass))
    {}

    explicit Framebuffer(std::array<sge::Ref<LLGL::Texture>, LLGL_MAX_NUM_COLOR_ATTACHMENTS> textures, sge::Ref<LLGL::RenderTarget> target, sge::Ref<LLGL::RenderPass> renderPass = nullptr) :
        m_textures(std::move(textures)),
        m_target(std::move(target)),
        m_render_pass(std::move(renderPass))
    {}

    [[nodiscard]]
    inline uint8_t ColorAttachmentCount() const noexcept {
        return m_target->GetNumColorAttachments();
    }

    [[nodiscard]]
    inline bool HasDepthAttachment() const noexcept {
        return m_target->HasDepthAttachment();
    }

    [[nodiscard]]
    inline bool HasStencilAttachment() const noexcept {
        return m_target->HasStencilAttachment();
    }

    [[nodiscard]]
    inline LLGL::Extent2D GetResolution() const noexcept {
        return m_target->GetResolution();
    }

    [[nodiscard]]
    inline const sge::Ref<LLGL::Texture>& GetTexture(uint8_t i) const noexcept {
        return m_textures[i];
    }

    [[nodiscard]]
    inline const sge::Ref<LLGL::RenderTarget>& GetRenderTarget() const noexcept {
        return m_target;
    }

private:
    std::array<sge::Ref<LLGL::Texture>, LLGL_MAX_NUM_COLOR_ATTACHMENTS> m_textures;
    sge::Ref<LLGL::RenderTarget> m_target;
    sge::Ref<LLGL::RenderPass> m_render_pass;
};

} // namespace sge

#endif // _SGE_TYPES_FRAMEBUFFER_HPP_