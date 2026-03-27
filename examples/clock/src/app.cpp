#include <SGE/engine.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/input.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/log.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/math/consts.hpp>

#include <chrono>
#include <glm/trigonometric.hpp>

#include "app.hpp"
#include "SGE/renderer/context.hpp"
#include "SGE/types/shape.hpp"

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

using namespace sge;

App::App(const ExampleConfig& config) : m_camera(config.backend, CameraOrigin::TopLeft) {
    InitRenderContext(config.backend);

    WindowSettings window_settings;
    window_settings.width = 800;
    window_settings.height = 800;
    window_settings.fullscreen = config.fullscreen;
    window_settings.vsync = config.vsync;
    window_settings.samples = config.samples;
    window_settings.hidden = true;

    auto result = CreateWindow(window_settings);
    if (!result.has_value()) {
        SGE_LOG_ERROR("Couldn't create a window: {}", result.error());
        std::abort();
    }

    GlfwWindow* window = result.value();
    m_primary_window_id = window->GetID();

    LLGL::Extent2D resolution = window->GetContentSize();
    m_camera.set_viewport({resolution.width, resolution.height});
    m_camera.set_zoom(1.0f);

    m_renderer = std::make_unique<Renderer>(GetRenderContext());

    m_batch = m_renderer->CreateBatch();
    m_batch->SetIsUi(true);

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    sync_time();

    window->ShowWindow();
}

void App::sync_time() {
    const std::chrono::time_zone* local_tz = std::chrono::current_zone();
    const std::chrono::time_point now = std::chrono::system_clock::now();
    const std::chrono::sys_info info = local_tz->get_info(now);

    m_t.time = Duration::Cast<Duration::Nanos>(now.time_since_epoch() + info.offset);
}

void App::OnUpdate() {
    if (Input::JustPressed(Key::Space)) {
        m_paused = !m_paused;
        if (!m_paused)
            sync_time();
    }

    if (Input::JustPressed(Key::W)) {
        auto _ = CreateWindow(WindowSettings {
            .width = 500,
            .height = 500,
            .samples = 1,
            .resizable = true,
            .vsync = true,
        });
    }

    if (m_paused) return;

    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = m_camera.zoom() * zoom_factor;

        m_camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = m_camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - m_camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const Rect& area = m_camera.get_projection_area();
        const glm::vec2 window_size = m_camera.viewport();

        const glm::vec2 new_position = m_camera.position() + deltaLength;
        m_camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size - area.size())));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const Rect& area = m_camera.get_projection_area();

        const glm::vec2 dir = glm::vec2(m_camera.right(), m_camera.down());

        const glm::vec2 new_position = m_camera.position() - Input::MouseDelta() * m_camera.zoom() * dir;
        m_camera.set_position(glm::clamp(new_position, -area.min, area.max));
        m_camera.set_position(new_position);
    }

    if (Input::JustPressed(Key::Escape)) {
        m_camera.set_position(glm::vec2(0.0f));
        m_camera.set_zoom(1.0f);
    }

    m_camera.update();

    m_t.time += Duration::Cast<Duration::Nanos>(Duration::SecondsFloat(Time::DeltaSeconds()));
    m_t.time = m_t.time % Duration::Hours(12);

    const float secs = static_cast<float>(m_t.time.count()) / static_cast<float>(std::nano::den);
    const float mins = secs / 60.0f;

    m_t.hours = mins / 60.0f;
    m_t.minutes = std::fmod(mins, 60.0f);
    m_t.seconds = std::fmod(secs, 60.0f);
}

void App::OnRender(const std::shared_ptr<GlfwWindow>& window) {
    LLGL::Extent2D resolution = window->GetContentSize();
    m_camera.set_viewport(glm::vec2(resolution.width, resolution.height));

    m_renderer->Begin();

    static constexpr float CLOCK_BORDER_WIDTH = 25.0f / 400.0f;
    static constexpr float CLOCK_HAND_THICKNESS = 9.0f / 800.0f;
    static constexpr float CLOCK_TICK_THICKNESS = 7.0f / 800.0f;
    static constexpr float CLOCK_FACE_PADDING = 20.0f / 400.0f;
    static constexpr float CLOCK_TICKS_LENGTH = 0.17f;
    static constexpr float CLOCK_HAND_OFFSET = 25.0f / 800.0f;
    static constexpr float CLOCK_SECOND_HAND_OFFSET = 0.02f;
    static constexpr float CLOCK_MINUTE_HAND_OFFSET = 0.065f;
    static constexpr float CLOCK_HOUR_HAND_OFFSET = 0.15f;
    static constexpr float CLOCK_CIRCLE_RADIUS = 30.0f / 800.0f;

    const glm::vec2 center = m_camera.screen_center();
    const glm::vec2 screen_size = glm::vec2(m_camera.viewport());

    glm::vec2 background_size = glm::vec2(screen_size);
    float aspect = background_size.x / background_size.y;
    background_size.y *= aspect;

    const float radius = background_size.y * 0.2f;

    m_batch->DrawRect(center, {
        .size = background_size,
        .color = sge::LinearRgba(0.0f, 0.0f, 0.0f),
        .border_thickness = CLOCK_BORDER_WIDTH * background_size.x / 2.0f,
        .border_color = sge::LinearRgba(0x3B, 0x40, 0x43),
        .border_radius = BorderRadius::Absolute(radius)
    });

    {
        const float padding = CLOCK_BORDER_WIDTH * background_size.x;
        glm::vec2 size = glm::vec2(screen_size - padding * 2.0f);
        const float aspect = size.x / size.y;
        size.y *= aspect;

        m_batch->DrawRect(center, {
            .size = size,
            .color = sge::LinearRgba(0x05, 0x0C, 0x0B),
            .border_radius = BorderRadius::Absolute(radius - padding)
        });

        m_batch->DrawCircle(center, {
            .radius = CLOCK_CIRCLE_RADIUS * size.x / 2.0f,
            .color = sge::LinearRgba::white(),
        });

        float tick_thickness = CLOCK_TICK_THICKNESS * size.x;
        float hand_thickness = CLOCK_HAND_THICKNESS * size.x;


        m_batch->BeginOrderMode();
        for (int i = 0; i < 4; ++i) {
            float t = ((float)i) / 4.0f;
            const float sin = glm::sin(t * 2.0f * consts::PI);
            const float cos = glm::cos(t * 2.0f * consts::PI);

            const glm::vec2 dir = glm::vec2(cos, sin);

            glm::vec2 start = center - dir * (size * 0.5f - CLOCK_FACE_PADDING * size * 0.5f - (size * CLOCK_TICKS_LENGTH) * 0.2f);
            glm::vec2 line_dir = dir * (size * CLOCK_TICKS_LENGTH - (size * CLOCK_TICKS_LENGTH) * 0.2f);

            m_batch->DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), BorderRadius::Relative(75.0f));
        }

        for (int i = 0; i < 12; ++i) {
            float t = ((float)i) / 12.0f;
            const float sin = glm::sin(t * 2.0f * consts::PI);
            const float cos = glm::cos(t * 2.0f * consts::PI);

            if (i % 3 == 0) continue;

            const glm::vec2 dir = glm::vec2(cos, sin);

            glm::vec2 start = center - dir * (size * 0.5f - CLOCK_FACE_PADDING * size * 0.5f + (size * CLOCK_TICKS_LENGTH) * 0.2f);
            glm::vec2 line_dir = dir * (size * CLOCK_TICKS_LENGTH);

            m_batch->DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), BorderRadius::Relative(75.0f));
        }
        m_batch->EndOrderMode();

        // m_batch->DrawCircle(center, {
        //     .radius = (size.x * 0.5f - CLOCK_FACE_PADDING * size.x * 0.5f + (size.x * CLOCK_TICKS_LENGTH) * 0.2f) - (size.x * CLOCK_TICKS_LENGTH) - (size.x * CLOCK_TICKS_LENGTH) * 0.2f,
        //     .color = sge::LinearRgba::transparent(),
        //     .border_thickness = 2.0f,
        //     .border_color = sge::LinearRgba(1.0f, 1.0f, 0.0f)
        // });
        // m_batch->DrawCircle(center, {
        //     .radius = (size.x * 0.5f - CLOCK_FACE_PADDING * size.x * 0.5f + (size.x * CLOCK_TICKS_LENGTH) * 0.2f) - (size.x * CLOCK_TICKS_LENGTH),
        //     .color = sge::LinearRgba::transparent(),
        //     .border_thickness = 2.0f,
        //     .border_color = sge::LinearRgba::blue()
        // });

        const float wh = m_t.hours / 12.0f * (2.0f * consts::PI);
        const float wm = m_t.minutes / 60.0f * (2.0f * consts::PI);
        const float ws = m_t.seconds / 60.0f * (2.0f * consts::PI);

        float hand_length = (size.x * 0.5f - CLOCK_FACE_PADDING * size.x * 0.5f + (size.x * CLOCK_TICKS_LENGTH) * 0.2f) - (size.x * CLOCK_TICKS_LENGTH) - (size.x * CLOCK_TICKS_LENGTH) * 0.2f + CLOCK_HAND_OFFSET * size.x;

        // Hour hand
        {
            const float sin = glm::sin(wh - consts::PI * 0.5f);
            const float cos = glm::cos(wh - consts::PI * 0.5f);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const float length = hand_length - CLOCK_HOUR_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), BorderRadius::Relative(75.0f));
        }

        // Minute hand
        {
            const float sin = glm::sin(wm - consts::PI * 0.5f);
            const float cos = glm::cos(wm - consts::PI * 0.5f);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const float length = hand_length - CLOCK_MINUTE_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), BorderRadius::Relative(75.0f));
        }

        // Second hand
        {
            const float sin = glm::sin(ws - consts::PI * 0.5f);
            const float cos = glm::cos(ws - consts::PI * 0.5f);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = center - line_dir * CLOCK_HAND_OFFSET * size.x;
            const float length = hand_length - CLOCK_SECOND_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba(0xDA, 0x30, 0x3B), BorderRadius::Relative(75.0f));
        }
    }

    m_renderer->BeginPass(window, m_camera);
        float red = ((float)0xC5) / 255.0f;
        float green = ((float)0xC8) / 255.0f;
        float blue = ((float)0xD3) / 255.0f;
        m_renderer->Clear(LLGL::ClearValue(red, green, blue, 1.0f));

        m_renderer->PrepareBatch(*m_batch);
        m_renderer->UploadBatchData();
        m_renderer->RenderBatch(*m_batch);

        m_batch->Reset();
    m_renderer->EndPass();

    m_renderer->End();
    GetRenderContext()->Present(window);
}

void App::OnPostRender(const std::shared_ptr<GlfwWindow>& window) {
#if SGE_DEBUG
    if (Input::Pressed(Key::C)) {
        GetRenderContext()->PrintDebugInfo();
    }
#endif
}

void App::OnWindowResized(const std::shared_ptr<GlfwWindow>& window, int width, int height) {
    m_camera.set_viewport(glm::uvec2(width, height));
    m_camera.update();
    OnRender(window);
}

App::~App() {
    m_renderer->DestroyBatch(*m_batch);
}
