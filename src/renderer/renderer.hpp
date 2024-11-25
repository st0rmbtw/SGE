#ifndef RENDERER_HPP
#define RENDERER_HPP

#pragma once

#include <LLGL/SwapChain.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Types.h>

#include "../types/sprite.hpp"
#include "../types/backend.hpp"
#include "../types/render_layer.hpp"
#include "../types/shape.hpp"
#include "../types/render_target.hpp"
#include "../assets.hpp"

#include "custom_surface.hpp"
#include "camera.h"

struct __attribute__((aligned(16))) ProjectionsUniform {
    glm::mat4 screen_projection_matrix;
    glm::mat4 view_projection_matrix;
    glm::mat4 transform_matrix;
    glm::vec2 camera_position;
    glm::vec2 window_size;
    float max_depth;
};

namespace Renderer {
    bool InitEngine(RenderBackend backend);
    bool Init(GLFWwindow* window, const LLGL::Extent2D& resolution, bool vsync, bool fullscreen);

    void Begin(const Camera& camera);
    void Render(const Camera& camera, LLGL::ClearValue clear = LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f, 0.0f));

    void DrawSprite(const Sprite& sprite, RenderLayer render_layer = RenderLayer::Main, int depth = -1, LLGL::RenderTarget* render_target = nullptr);
    inline void DrawSprite(const Sprite& sprite, int depth, LLGL::RenderTarget* render_target = nullptr) {
        DrawSprite(sprite, RenderLayer::Main, depth, render_target);
    }
    
    void DrawAtlasSprite(const TextureAtlasSprite& sprite, RenderLayer render_layer = RenderLayer::Main, int depth = -1);
    inline void DrawAtlasSprite(const TextureAtlasSprite& sprite, int depth) {
        DrawAtlasSprite(sprite, RenderLayer::Main, depth);
    }

    void DrawSpriteUI(const Sprite& sprite, int depth = -1);
    void DrawAtlasSpriteUI(const TextureAtlasSprite& sprite, int depth = -1);

    void DrawShape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, float border_radius = 0.0f, Anchor anchor = Anchor::Center, bool is_ui = false, int depth = -1);

    inline void DrawCircle(glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, Anchor anchor = Anchor::Center, bool is_ui = false, int depth = -1) {
        DrawShape(Shape::Circle, position, size, color, border_color, border_thickness, 0.0, anchor, is_ui, depth);
    }

    inline void DrawRect(glm::vec2 position, glm::vec2 size, const glm::vec4& color, const glm::vec4& border_color, float border_thickness, float border_radius = 0.0f, Anchor anchor = Anchor::Center, bool is_ui = false, int depth = -1) {
        DrawShape(Shape::Rect, position, size, color, border_color, border_thickness, border_radius, anchor, is_ui, depth);
    }

    inline void DrawShape(Shape::Type shape, glm::vec2 position, glm::vec2 size, const glm::vec4& color, float border_radius = 0.0f, Anchor anchor = Anchor::Center, bool is_ui = false, int depth = -1) {
        DrawShape(shape, position, size, color, color, 0.0f, border_radius, anchor, is_ui, depth);
    }

    void DrawText(const char* text, uint32_t length, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, bool is_ui = false, int depth = -1, LLGL::RenderTarget* render_target = nullptr);

    inline void DrawText(const std::string& text, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, LLGL::RenderTarget* render_target = nullptr) {
        DrawText(text.c_str(), text.length(), size, position, color, font, false, -1, render_target);
    }
    inline void DrawTextUi(const char* text, uint32_t length, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, int depth = -1, LLGL::RenderTarget* render_target = nullptr) {
        DrawText(text, length, size, position, color, font, true, depth, render_target);
    }
    inline void DrawTextUi(const std::string& text, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, int depth = -1, LLGL::RenderTarget* render_target = nullptr) {
        DrawText(text.c_str(), text.length(), size, position, color, font, true, depth, render_target);
    }

    inline void DrawChar(char ch, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, int depth = -1, LLGL::RenderTarget* render_target = nullptr) {
        DrawText(&ch, 1, size, position, color, font, false, depth, render_target);
    }
    inline void DrawCharUi(char ch, float size, const glm::vec2& position, const glm::vec3& color, FontAsset font, int depth = -1, LLGL::RenderTarget* render_target = nullptr) {
        DrawText(&ch, 1, size, position, color, font, true, depth, render_target);
    }

    void Clear(LLGL::RenderTarget* render_target, long clear_flags, const LLGL::ClearValue& clear_value);

    RenderTarget CreateRenderTarget(LLGL::Extent2D resolution);

    void ResizeRenderTarget(RenderTarget* render_target, LLGL::Extent2D new_size);
    
    void Release(RenderTarget* render_target);

#if DEBUG
    void PrintDebugInfo();
#endif

    void Terminate();

    [[nodiscard]] const LLGL::RenderSystemPtr& Context();
    [[nodiscard]] LLGL::SwapChain* SwapChain();
    [[nodiscard]] LLGL::CommandBuffer* CommandBuffer();
    [[nodiscard]] LLGL::CommandQueue* CommandQueue();
    [[nodiscard]] const std::shared_ptr<CustomSurface>& Surface();
    [[nodiscard]] LLGL::Buffer* GlobalUniformBuffer();
    [[nodiscard]] RenderBackend Backend();
    [[nodiscard]] uint32_t GetMainDepthIndex();
    [[nodiscard]] uint32_t GetUiDepthIndex();
};

#endif