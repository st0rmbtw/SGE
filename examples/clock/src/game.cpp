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
#include <SGE/log.hpp>
#include <SGE/time/stopwatch.hpp>

#include <chrono>
#include <glm/trigonometric.hpp>

#include "constants.hpp"

using namespace sge;

struct CurrentTime {
    Duration::Millis time;
    float seconds;
    float minutes;
    float hours;
};

static struct GameState {
    Batch batch;
    Camera camera = Camera(CameraOrigin::TopLeft);
    bool paused = false;
    CurrentTime t;
} g;

void pre_update() {

}

void fixed_update() {

}

void update() {
    if (Input::JustPressed(Key::Space)) g.paused = !g.paused;
    if (g.paused) return;
    
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

    g.t.time += Duration::Cast<Duration::Millis>(Duration::SecondsFloat(Time::DeltaSeconds()));
    g.t.time = g.t.time % Duration::Hours(12);

    const float secs = static_cast<float>(g.t.time.count()) / 1000.0f;
    const float mins = secs / 60.0f;

    g.t.hours = mins / 60.0f;
    g.t.minutes = std::fmod(mins, 60.0f);
    g.t.seconds = std::fmod(secs, 60.0f);
}

void post_update() {

}

void render() {
    Renderer& renderer = Engine::Renderer();

    renderer.Begin(g.camera);

    const glm::vec2 center = g.camera.screen_center();
    const glm::vec2 screen_size = glm::vec2(g.camera.viewport());

    {
        glm::vec2 size = glm::vec2(screen_size);
        float aspect = size.x / size.y;
        size.y *= aspect;

        g.batch.DrawRect(center, size, sge::LinearRgba(0.0f, 0.0f, 0.0f), sge::LinearRgba(0x3B, 0x40, 0x43), 16.0f, glm::vec4(size.y * 0.2f + 1.0f));
    }
    {
        glm::vec2 size = glm::vec2(screen_size - 32.0f - 40.0f);
        const float aspect = size.x / size.y;
        size.y *= aspect;

        g.batch.DrawRect(center, size, sge::LinearRgba(0x06, 0x0D, 0x0D), sge::LinearRgba::black(), 0.0f, glm::vec4(size.y * 0.2f));

        g.batch.DrawCircle(center, glm::vec2(20.0f), sge::LinearRgba::white(), sge::LinearRgba::white(), 0.0f);

        constexpr float CLOCK_HAND_THICKNESS = 8.0f;
        constexpr float CLOCK_TICK_THICKNESS = 5.0f;
        constexpr float CLOCK_FACE_PADDING = 20.0f;
        constexpr float CLOCK_TICKS_LENGTH = 0.15f;
        constexpr float CLOCK_SECOND_HAND_OFFSET = 25.0f;

        for (int i = 0; i < 12; ++i) {
            float t = ((float)i) / 12.0f;
            const float sin = glm::sin(t * 2.0f * glm::pi<float>());
            const float cos = glm::cos(t * 2.0f * glm::pi<float>());

            glm::vec2 start = center - glm::normalize(glm::vec2(cos, sin)) * glm::vec2(size * 0.5f - CLOCK_FACE_PADDING);
            glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin)) * glm::vec2(size * CLOCK_TICKS_LENGTH);

            g.batch.DrawLine(start, start + line_dir, CLOCK_TICK_THICKNESS, sge::LinearRgba(0xFF, 0xFF, 0xFF));
        }
        
        const float wh = g.t.hours / 12.0f * (2.0 * glm::pi<float>());
        const float wm = g.t.minutes / 60.0f * (2.0 * glm::pi<float>());
        const float ws = g.t.seconds / 60.0f * (2.0 * glm::pi<float>());

        // Hour hand
        {
            const float sin = glm::sin(wh - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(wh - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_SECOND_HAND_OFFSET);
            const glm::vec2 length = glm::vec2(size * (0.5f - CLOCK_TICKS_LENGTH) - CLOCK_FACE_PADDING - 15.0f + CLOCK_SECOND_HAND_OFFSET);

            g.batch.DrawLine(start, start + line_dir * length, CLOCK_HAND_THICKNESS, sge::LinearRgba::white());
        }

        // Minute hand
        {
            const float sin = glm::sin(wm - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(wm - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_SECOND_HAND_OFFSET);
            const glm::vec2 length = glm::vec2(size * (0.5f - CLOCK_TICKS_LENGTH) - CLOCK_FACE_PADDING - 15.0f + CLOCK_SECOND_HAND_OFFSET);

            g.batch.DrawLine(start, start + line_dir * length, CLOCK_HAND_THICKNESS, sge::LinearRgba::blue());
        }

        // Second hand
        {
            const float sin = glm::sin(ws - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(ws - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_SECOND_HAND_OFFSET);
            const glm::vec2 length = glm::vec2(size * (0.5f - CLOCK_TICKS_LENGTH) - CLOCK_FACE_PADDING - 15.0f + CLOCK_SECOND_HAND_OFFSET);

            g.batch.DrawLine(start, start + line_dir * length, CLOCK_HAND_THICKNESS, sge::LinearRgba(0xFF, 0x00, 0x25));
        }
    }

    renderer.BeginMainPass();
        float red = ((float)0xC5) / 255.0f;
        float green = ((float)0xC8) / 255.0f;
        float blue = ((float)0xD3) / 255.0f;
        renderer.Clear(LLGL::ClearValue(red, green, blue, 1.0f));

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

void window_resized(uint32_t width, uint32_t height, uint32_t, uint32_t) {
    g.camera.set_viewport(glm::uvec2(width, height));
    g.camera.update();
    render();
}

bool Game::Init(RenderBackend backend, AppConfig config) {
    Engine::SetPreUpdateCallback(pre_update);
    Engine::SetUpdateCallback(update);
    Engine::SetPostUpdateCallback(post_update);
    Engine::SetFixedUpdateCallback(fixed_update);
    Engine::SetRenderCallback(render);
    Engine::SetPostRenderCallback(post_render);
    Engine::SetWindowResizeCallback(window_resized);

    glm::uvec2 window_size = glm::uvec2(800, 800);

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

    g.batch.SetIsUi(true);
    g.batch.BeginBlendMode(sge::BlendMode::PremultipliedAlpha);

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    g.t.time = Duration::Cast<Duration::Millis>(std::chrono::seconds(t + now->tm_gmtoff));

    return true;
}

void Game::Run() {
    Engine::Run();
}

void Game::Destroy() {
    Engine::Destroy();
}
