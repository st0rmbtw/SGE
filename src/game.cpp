#include "game.hpp"
#include "constants.hpp"
#include "engine.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.h"
#include "input.hpp"
#include "time/time.hpp"
#include "ui.hpp"

static struct GameState {
    Camera camera;
} g;

bool load_assets() {
    if (!Assets::Load()) return false;
    if (!Assets::LoadFonts()) return false;
    if (!Assets::InitSamplers()) return false;
    if (!Assets::LoadShaders()) return false;

    return true;
}

void window_resized(uint32_t width, uint32_t height) {
    g.camera.set_viewport(glm::uvec2(width, height));
}

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

    if (g.camera.needs_update()) {
        g.camera.update();
    }
}

void post_update() {

}

void render() {
    Renderer::Begin(g.camera);

    Sprite sprite;
    sprite.set_custom_size(glm::vec2(50.0f));
    sprite.set_color(glm::vec3(1.0f, 0.0f, 0.0f));
    Renderer::DrawSprite(sprite);

    Renderer::DrawShape(Shape::Circle, glm::vec2(g.camera.viewport()) / 2.0f, glm::vec2(100.0f),glm::vec4(0.5f, 0.93f, 0.5f, 1.0f), glm::vec4(1.0f), 0.1f);

    Renderer::Render(g.camera);
}

void post_render() {
    g.camera.set_mutated(false);

#if DEBUG
    if (Input::Pressed(Key::C)) {
        Renderer::PrintDebugInfo();
    }
#endif
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

    glm::uvec2 window_size = glm::uvec2(1280, 720);
    if (!Engine::Init(backend, config.vsync, config.fullscreen, window_size.x, window_size.y, true)) return false;

    Time::set_fixed_timestep_seconds(Constants::FIXED_UPDATE_INTERVAL);
    
    g.camera.set_viewport({window_size.x, window_size.y});
    g.camera.set_zoom(1.0f);

    UI::Init();

    Engine::ShowWindow();

    return true;
}

void Game::Run() {
    Engine::Run();
}

void Game::Destroy() {
    Engine::Destroy();
}