#include "game.hpp"
#include "engine/engine.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/renderer/camera.hpp"
#include "engine/input.hpp"
#include "engine/time/time.hpp"
#include "engine/types/anchor.hpp"
#include "engine/types/color.hpp"
#include "engine/types/window_settings.hpp"

#include "utils.hpp"

#include "assets.hpp"

#include "constants.hpp"

using namespace sge;
using namespace sge::renderer;
using namespace sge::renderer::batch;
using namespace sge::input;
using namespace sge::types;
using namespace sge::time;

static struct GameState {
    Batch batch;
    Camera camera = Camera(CameraOrigin::TopLeft);
} g;

static std::array ICONS = {
    TextureAsset::IconFinder,
    TextureAsset::IconLaunchpad,
    TextureAsset::IconMessage,
    TextureAsset::IconMail,
    TextureAsset::IconMaps,
    TextureAsset::IconPhotos,
    TextureAsset::IconCalendar,
    TextureAsset::IconContacts,
    TextureAsset::IconReminders,
    TextureAsset::IconNotes,
    TextureAsset::IconMusic,
    TextureAsset::IconAppStore,
    TextureAsset::IconPreferences,
    TextureAsset::IconBin
};

void pre_update() {

}

void fixed_update() {

}

void update() {
    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = g.camera.zoom() * zoom_factor;
        
        g.camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = g.camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - g.camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const math::Rect area = g.camera.get_projection_area();
        const glm::vec2 window_size = g.camera.viewport();

        const glm::vec2 new_position = g.camera.position() + deltaLength;
        g.camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size - area.size())));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const math::Rect area = g.camera.get_projection_area();
        const glm::vec2 window_size = g.camera.viewport();

        const glm::vec2 new_position = g.camera.position() - Input::MouseDelta() * g.camera.zoom();
        g.camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size - area.size())));
    }

    if (Input::JustPressed(Key::Escape)) {
        g.camera.set_position(glm::vec2(0.0f));
    }

    g.camera.update();
}

void post_update() {

}

void render() {
    Renderer& renderer = Engine::Renderer();

    renderer.Begin(g.camera);
        const glm::vec2 window_size = g.camera.viewport();

        Sprite sprite(Assets::GetTexture(TextureAsset::DesktopBackground));
        sprite.set_position(glm::vec2(0.0f));
        sprite.set_custom_size(glm::vec2(window_size.x, window_size.y));
        sprite.set_anchor(Anchor::TopLeft);
        g.batch.DrawSprite(sprite);

        const glm::vec2 top_bar_size = glm::vec2(window_size.x, 24.0f);

        const glm::vec2 icon_size = glm::vec2(52.0f);
        const float icon_margin = 0.0f;
        const glm::vec2 dock_padding = glm::vec2(4.0f, 4.0f);
        const glm::vec2 dock_size = glm::vec2(dock_padding.x * 2.0f + ICONS.size() * icon_size.x + (ICONS.size() - 1) * icon_margin, icon_size.y + dock_padding.y * 2.0f);
        const glm::vec2 dock_pos = glm::vec2(window_size.x / 2.0f - dock_size.x / 2.0f, window_size.y - dock_size.y - 6.0f);
        
        g.batch.BeginOrderMode();
            // const color::LinearRgba top_bar_color = color::LinearRgba(108, 125, 141, 0.44f);
            const color::LinearRgba top_bar_color = color::LinearRgba(246, 246, 246, 0.5f);
            g.batch.DrawRect(glm::vec2(window_size.x / 2.0f, 0.0f), top_bar_size, top_bar_color, color::LinearRgba(0.0f), 0.0f, 0.0f, Anchor::TopCenter, true);
            
            // const glm::vec4 dock_color = glm::vec4(108.0f / 255.0f, 125.0f / 255.0f, 141.0f / 255.0f, 0.44f);
            const color::LinearRgba dock_color = color::LinearRgba(1.0f, 0.15f);
            const color::LinearRgba dock_border_color = color::LinearRgba(1.0f, dock_color.alpha + 0.15f);
            g.batch.DrawRect(dock_pos, dock_size, dock_color, dock_border_color, 1.0f, 20.0f, Anchor::TopLeft, true);
        g.batch.EndOrderMode();

        float icon_x = dock_pos.x + dock_padding.x;
        float icon_y = dock_pos.y + dock_padding.y;

        g.batch.BeginOrderMode();
            for (const TextureAsset icon : ICONS) {
                Sprite sprite(Assets::GetTexture(icon));
                sprite.set_position(glm::vec2(icon_x, icon_y));
                sprite.set_custom_size(icon_size);
                sprite.set_anchor(Anchor::TopLeft);
                g.batch.DrawSprite(sprite);

                icon_x += icon_size.x + icon_margin;
            }
        g.batch.EndOrderMode();

    renderer.BeginMainPass(LLGL::ClearValue(0.0f, 0.0f, 0.0f, 0.0f));
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
    g.camera.update();
    render();
}

void destroy() {
    Renderer& renderer = Engine::Renderer();
    Assets::DestroyTextures(renderer);
    Assets::DestroySamplers(renderer);
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

    // g.batch.set_is_ui(true);

    return true;
}

void Game::Run() {
    Engine::Run();
}

void Game::Destroy() {
    Engine::Destroy();
}