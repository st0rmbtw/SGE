#include "app.hpp"

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
    Duration::Nanos time;
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

void sync_time() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    g.t.time = Duration::Cast<Duration::Nanos>(std::chrono::seconds(t + now->tm_gmtoff));
}

void update() {
    if (Input::JustPressed(Key::Space)) {
        g.paused = !g.paused;
        if (!g.paused) 
            sync_time();
    }
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

    g.t.time += Duration::Cast<Duration::Nanos>(Duration::SecondsFloat(Time::DeltaSeconds()));
    g.t.time = g.t.time % Duration::Hours(12);

    const float secs = static_cast<float>(g.t.time.count()) / static_cast<float>(std::nano::den);
    const float mins = secs / 60.0f;

    g.t.hours = mins / 60.0f;
    g.t.minutes = std::fmod(mins, 60.0f);
    g.t.seconds = std::fmod(secs, 60.0f);
}

void post_update() {

}

static float ApproxEquals(float a, float b) {
    return std::abs(a-b) < 0.01f;
}

void render() {
    Renderer& renderer = Engine::Renderer();

    renderer.Begin(g.camera);

    static constexpr float CLOCK_BORDER_WIDTH = 25.0f / 400.0f;
    static constexpr float CLOCK_HAND_THICKNESS = 9.0f / 800.0f;
    static constexpr float CLOCK_TICK_THICKNESS = 7.0f / 800.0f;
    static constexpr float CLOCK_FACE_PADDING = 20.0f / 400.0f;
    static constexpr float CLOCK_TICKS_LENGTH = 0.17f;
    static constexpr float CLOCK_HAND_OFFSET = 25.0f / 800.0f;
    static constexpr float CLOCK_SECOND_HAND_LENGTH = 1.0f * 0.5f;
    static constexpr float CLOCK_MINUTE_HAND_LENGTH = 0.9f * 0.5f;
    static constexpr float CLOCK_HOUR_HAND_LENGTH = 0.75f * 0.5f;
    static constexpr float CLOCK_CIRCLE_RADIUS = 30.0f / 800.0f;

    const glm::vec2 center = g.camera.screen_center();
    const glm::vec2 screen_size = glm::vec2(g.camera.viewport());

    glm::vec2 background_size = glm::vec2(screen_size);
    float aspect = background_size.x / background_size.y;
    background_size.y *= aspect;

    const float radius = background_size.y * 0.2f;

    {
        g.batch.DrawRect(center, background_size, sge::LinearRgba(0.0f, 0.0f, 0.0f), sge::LinearRgba(0x3B, 0x40, 0x43), CLOCK_BORDER_WIDTH * background_size.x / 2.0f, glm::vec4(radius));
    }
    {
        const float padding = CLOCK_BORDER_WIDTH * background_size.x;
        glm::vec2 size = glm::vec2(screen_size - padding * 2.0f);
        const float aspect = size.x / size.y;
        size.y *= aspect;

        g.batch.DrawRect(center, size, sge::LinearRgba(0x05, 0x0C, 0x0B), sge::LinearRgba::black(), 0.0f, glm::vec4(radius - padding));

        g.batch.DrawCircle(center, glm::vec2(CLOCK_CIRCLE_RADIUS * size.x), sge::LinearRgba::white(), sge::LinearRgba::white(), 0.0f);

        float tick_thickness = CLOCK_TICK_THICKNESS * size.x;
        float hand_thickness = CLOCK_HAND_THICKNESS * size.x;
        
        for (int i = 0; i < 4; ++i) {
            float t = ((float)i) / 4.0f;
            const float sin = glm::sin(t * 2.0f * glm::pi<float>());
            const float cos = glm::cos(t * 2.0f * glm::pi<float>());

            const glm::vec2 dir = glm::normalize(glm::vec2(cos, sin));

            glm::vec2 start = center - dir * (size * 0.5f - CLOCK_FACE_PADDING * size * 0.5f - (size * CLOCK_TICKS_LENGTH) * 0.2f);
            glm::vec2 line_dir = dir * (size * CLOCK_TICKS_LENGTH - (size * CLOCK_TICKS_LENGTH) * 0.2f);

            g.batch.DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), glm::vec4(tick_thickness / 2.0f));
        }

        for (int i = 0; i < 12; ++i) {
            float t = ((float)i) / 12.0f;
            const float sin = glm::sin(t * 2.0f * glm::pi<float>());
            const float cos = glm::cos(t * 2.0f * glm::pi<float>());

            if (i % 3 == 0) continue;

            const glm::vec2 dir = glm::normalize(glm::vec2(cos, sin));

            glm::vec2 start = center - dir * (size * 0.5f - CLOCK_FACE_PADDING * size * 0.5f + (size * CLOCK_TICKS_LENGTH) * 0.2f);
            glm::vec2 line_dir = dir * (size * CLOCK_TICKS_LENGTH);

            g.batch.DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), glm::vec4(tick_thickness / 2.0f));
        }

        // g.batch.DrawCircle(center, glm::vec2((size * 0.5f) + (size * CLOCK_TICKS_LENGTH)), sge::LinearRgba::transparent(), LinearRgba(1.0f, 1.0f, 0.0f), 2.0f);

        // g.batch.DrawCircle(center, glm::vec2((size * 0.5f) + (size * CLOCK_TICKS_LENGTH) - (size * CLOCK_TICKS_LENGTH) * 0.2f), sge::LinearRgba::transparent(), sge::LinearRgba::blue(), 2.0f);
        
        const float wh = g.t.hours / 12.0f * (2.0 * glm::pi<float>());
        const float wm = g.t.minutes / 60.0f * (2.0 * glm::pi<float>());
        const float ws = g.t.seconds / 60.0f * (2.0 * glm::pi<float>());

        // Hour hand
        {
            const float sin = glm::sin(wh - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(wh - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const glm::vec2 length = size * (CLOCK_HOUR_HAND_LENGTH - CLOCK_TICKS_LENGTH - CLOCK_TICKS_LENGTH * 0.2f) - CLOCK_FACE_PADDING * size.x / 2.0f - 15.0f + CLOCK_HAND_OFFSET;

            g.batch.DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), glm::vec4(hand_thickness / 2.0f));
        }

        // Minute hand
        {
            const float sin = glm::sin(wm - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(wm - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const glm::vec2 length = size * (CLOCK_MINUTE_HAND_LENGTH - CLOCK_TICKS_LENGTH - CLOCK_TICKS_LENGTH * 0.2f) - CLOCK_FACE_PADDING * size.x / 2.0f - 15.0f + CLOCK_HAND_OFFSET;

            g.batch.DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), glm::vec4(hand_thickness / 2.0f));
        }

        // Second hand
        {
            const float sin = glm::sin(ws - glm::pi<float>() * 0.5f);
            const float cos = glm::cos(ws - glm::pi<float>() * 0.5f);
            const glm::vec2 line_dir = glm::normalize(glm::vec2(cos, sin));

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const glm::vec2 length = glm::vec2(size * 0.5f - size * 0.5f * (CLOCK_TICKS_LENGTH + CLOCK_TICKS_LENGTH * 0.2f) - padding * 0.5f - 15.0f + CLOCK_HAND_OFFSET);

            g.batch.DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba(0xDA, 0x30, 0x3B), glm::vec4(hand_thickness / 2.0f));
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

bool App::Init(RenderBackend backend, AppConfig config) {
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

    sync_time();
    sync_time();

    return true;
}

void App::Run() {
    Engine::Run();
}

void App::Destroy() {
    Engine::Destroy();
}
