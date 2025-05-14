#include "game.hpp"

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/input.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/blend_mode.hpp>

#include <glm/trigonometric.hpp>

#include "assets.hpp"
#include "constants.hpp"

#define CLAY_IMPLEMENTATION
#include "clay.h"

using namespace sge;

static struct GameState {
    Batch batch;
    Camera camera = Camera(CameraOrigin::TopLeft);
} g;

void pre_update() {

}

void fixed_update() {

}

void update() {
    Clay_SetLayoutDimensions(Clay_Dimensions {
        .width = static_cast<float>(g.camera.viewport().x),
        .height = static_cast<float>(g.camera.viewport().y)
    });
    
    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = g.camera.zoom() * zoom_factor;
        
        g.camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = g.camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - g.camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const Rect& area = g.camera.get_projection_area();
        const glm::vec2 window_size = g.camera.viewport();

        const glm::vec2 new_position = g.camera.position() + deltaLength;
        g.camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size - area.size())));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const Rect& area = g.camera.get_projection_area();
        const glm::vec2 half_screen_size = glm::vec2(g.camera.viewport()) / 2.0f;

        const glm::vec2 dir = glm::vec2(g.camera.right(), g.camera.down());

        const glm::vec2 new_position = g.camera.position() - Input::MouseDelta() * g.camera.zoom() * dir;
        g.camera.set_position(glm::clamp(new_position, -area.min, area.max));
        g.camera.set_position(new_position);
    }

    if (Input::JustPressed(Key::Escape)) {
        g.camera.set_position(glm::vec2(0.0f));
        g.camera.set_zoom(1.0f);
    }

    g.camera.update();
}

void post_update() {

}

void render() {
    Renderer& renderer = Engine::Renderer();

    renderer.Begin(g.camera);

    Clay_BeginLayout();

    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 10, 10, 10, 10 },
            .childGap = 0,
        },
        .backgroundColor = { 25, 25, 25, 255 }
    }) {
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            },
            .backgroundColor = { 255, 0, 0, 255 },
            .cornerRadius = { 12, 0, 12, 0 },
        });

        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            },
            .backgroundColor = { 0, 255, 0, 255 },
            .cornerRadius = { 0, 12, 0, 12 },
        });
    }

    Clay_RenderCommandArray drawCommands = Clay_EndLayout();
    for (int i = 0; i < drawCommands.length; ++i) {
        Clay_RenderCommand* drawCommand = &drawCommands.internalArray[i];
        Clay_BoundingBox boundingBox = drawCommand->boundingBox;

        switch (drawCommand->commandType) {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleRenderData *config = &drawCommand->renderData.rectangle;
            const glm::vec2 position = glm::vec2(boundingBox.x, boundingBox.y);
            const glm::vec2 size = glm::vec2(boundingBox.width, boundingBox.height);
            const LinearRgba color = LinearRgba(config->backgroundColor.r / 255.0f, config->backgroundColor.g / 255.0f, config->backgroundColor.b / 255.0f, config->backgroundColor.a / 255.0f);
            const glm::vec4 cornerRadius = glm::vec4(config->cornerRadius.topLeft, config->cornerRadius.topRight, config->cornerRadius.bottomLeft, config->cornerRadius.bottomRight);

            g.batch.DrawRect(position, size, color, color, 0.0f, cornerRadius, Anchor::TopLeft);
        } break;
        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderRenderData *config = &drawCommand->renderData.border;
            
            const LinearRgba color = LinearRgba(config->color.r / 255.0f, config->color.g / 255.0f, config->color.b / 255.0f, config->color.a / 255.0f);

            const glm::vec4 cornerRadius = glm::vec4(0.0f);

            // Left border
            if (config->width.left > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x, boundingBox.y + config->cornerRadius.topLeft);
                const glm::vec2 size = glm::vec2(config->width.left, boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft);

                g.batch.DrawRect(position, size, color, LinearRgba::black(), 0.0f, cornerRadius, Anchor::TopLeft);
            }
            // Right border
            if (config->width.right > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->width.right, boundingBox.y + config->cornerRadius.topRight);
                const glm::vec2 size = glm::vec2(config->width.right, boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight);

                g.batch.DrawRect(position, size, color, LinearRgba::black(), 0.0f, cornerRadius, Anchor::TopLeft);
            }
            // Top border
            if (config->width.top > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.topLeft, boundingBox.y);
                const glm::vec2 size = glm::vec2(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight, config->width.top);

                g.batch.DrawRect(position, size, color, LinearRgba::black(), 0.0f, cornerRadius, Anchor::TopLeft);
            }

            // Bottom border
            if (config->width.bottom > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->width.bottom);
                const glm::vec2 size = glm::vec2(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight, config->width.bottom);

                g.batch.DrawRect(position, size, color, LinearRgba::black(), 0.0f, cornerRadius, Anchor::TopLeft);
            }

            if (config->cornerRadius.topLeft > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.topLeft, boundingBox.y + config->cornerRadius.topLeft);
                g.batch.DrawArc(position, config->cornerRadius.topLeft, config->cornerRadius.topLeft - config->width.top, color, glm::radians(90.0f), glm::radians(180.0f), Anchor::Center);
            }

            if (config->cornerRadius.topRight > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->cornerRadius.topRight, boundingBox.y + config->cornerRadius.topRight);
                g.batch.DrawArc(position, config->cornerRadius.topRight, config->cornerRadius.topRight - config->width.top, color, glm::radians(0.0f), glm::radians(90.0f), Anchor::Center);
            }

            if (config->cornerRadius.bottomLeft > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft);
                g.batch.DrawArc(position, config->cornerRadius.bottomLeft, config->cornerRadius.bottomLeft - config->width.bottom, color, glm::radians(180.0f), glm::radians(270.0f), Anchor::Center);
            }

            if (config->cornerRadius.bottomRight > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight, boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight);
                g.batch.DrawArc(position, config->cornerRadius.bottomRight, config->cornerRadius.bottomRight - config->width.bottom, color, glm::radians(270.0f), glm::radians(360.0f), Anchor::Center);
            }
        }
        break;
        case CLAY_RENDER_COMMAND_TYPE_NONE:
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
        case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
          break;
        }
    }

    const glm::vec2 center = g.camera.screen_center();
    g.batch.DrawLine(center, center + glm::vec2(100.0), 2.0, sge::LinearRgba::blue());

    renderer.BeginMainPass();
        renderer.Clear(LLGL::ClearValue(0.0f, 0.0f, 0.0f, 0.0f));

        renderer.PrepareBatch(g.batch);
        renderer.UploadBatchData();
        renderer.RenderBatch(g.batch);

        g.batch.Reset();
    renderer.EndPass();
    
    renderer.End();
}

void post_render() {
#if DEBUG
    Renderer& renderer = Engine::Renderer();

    if (Input::Pressed(Key::C)) {
        renderer.PrintDebugInfo();
    }
#endif
}

bool load_assets() {
    Renderer& renderer = Engine::Renderer();

    if (!Assets::LoadTextures(renderer)) return false;
    if (!Assets::LoadFonts(renderer)) return false;

    return true;
}

void window_resized(uint32_t width, uint32_t height, uint32_t, uint32_t) {
    g.camera.set_viewport(glm::uvec2(width, height));
    g.camera.update();
    render();
}

void destroy() {
    Renderer& renderer = Engine::Renderer();
    Assets::DestroyTextures(renderer);
    Assets::DestroySamplers(renderer);
}

static void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

bool Game::Init(RenderBackend backend, AppConfig config) {
    Engine::SetLoadAssetsCallback(load_assets);
    Engine::SetPreUpdateCallback(pre_update);
    Engine::SetUpdateCallback(update);
    Engine::SetPostUpdateCallback(post_update);
    Engine::SetFixedUpdateCallback(fixed_update);
    Engine::SetRenderCallback(render);
    Engine::SetPostRenderCallback(post_render);
    Engine::SetWindowResizeCallback(window_resized);
    Engine::SetDestroyCallback(destroy);

    glm::uvec2 window_size = glm::uvec2(1280, 720);

    WindowSettings settings;
    settings.width = window_size.x;
    settings.height = window_size.y;
    settings.fullscreen = config.fullscreen;
    settings.hidden = true;

    LLGL::Extent2D resolution;
    if (!Engine::Init(backend, config.vsync, settings, resolution)) return false;

    Time::SetFixedTimestepSeconds(Constants::FIXED_UPDATE_INTERVAL);
    
    g.camera.set_viewport({resolution.width, resolution.height});
    g.camera.set_zoom(1.0f);

    Engine::ShowWindow();
    
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char *)malloc(totalMemorySize));
    Clay_Initialize(
        clayMemory,
        Clay_Dimensions{ static_cast<float>(resolution.width), static_cast<float>(resolution.height) },
        Clay_ErrorHandler{ .errorHandlerFunction = HandleClayErrors, .userData = NULL }
    );

    g.batch.SetIsUi(true);
    g.batch.BeginBlendMode(sge::BlendMode::PremultipliedAlpha);

    return true;
}

void Game::Run() {
    Engine::Run();
}

void Game::Destroy() {
    Engine::Destroy();
}
