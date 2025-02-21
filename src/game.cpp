#include "game.hpp"

#include <LLGL/Types.h>

#include "engine/engine.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/renderer/camera.hpp"
#include "engine/input.hpp"
#include "engine/time/time.hpp"
#include "engine/types/window_settings.hpp"

#include "assets.hpp"
#include "constants.hpp"

static struct GameState {
    Camera camera;
    Batch batch;
} g;

void pre_update() {

}

void fixed_update() {

}

void update() {
    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);

        const glm::vec2 mouse_pos = g.camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - g.camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        g.camera.set_position(g.camera.position() + deltaLength);
        g.camera.set_zoom(g.camera.zoom() * zoom_factor);
    }

    if (Input::Pressed(MouseButton::Left)) {
        g.camera.set_position(g.camera.position() - Input::MouseDelta() * g.camera.zoom());
    }

    g.camera.update();
}

void post_update() {

}

void render() {
    Renderer& renderer = Engine::Renderer();

    renderer.Begin(g.camera);
    renderer.BeginMainPass(LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.0f));

    g.batch.DrawCircle(glm::vec2(0.0f), glm::vec2(100.0f), glm::vec4(0.5f, 0.93f, 0.5f, 1.0f), glm::vec4(1.0f), 0.1f);
    // g.batch.DrawRect(glm::vec2(0.0f), glm::vec2(100.0f, 100.0f), glm::vec4(0.5f, 0.93f, 0.5f, 1.0f), glm::vec4(1.0f), 1.0f, 10.0f, Anchor::TopLeft);

    renderer.PrepareBatch(g.batch);
    renderer.UploadBatchData();
    renderer.RenderBatch(g.batch);

    g.batch.Reset();

    renderer.EndMainPass();
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
}

void destroy() {
}

bool Game::Init(RenderBackend backend, GameConfig config) {
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

    return true;
}

void Game::Run() {
    Engine::Run();
}

void Game::Destroy() {
    Engine::Destroy();
}