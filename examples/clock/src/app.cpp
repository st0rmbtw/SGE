#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/math/consts.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/window_settings.hpp>

#include <chrono>
#include <glm/trigonometric.hpp>
#include <imgui.h>

#include "app.hpp"

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

namespace Input = sge::Input;
namespace Time = sge::Time;
namespace Duration = sge::Duration;
namespace consts = sge::consts;
using Key = sge::Key;
using MouseButton = sge::MouseButton;

bool App::OnInit() {
    sge::ImGuiConfig imguiConfig;
    imguiConfig.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiConfig.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // imguiConfig.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if (!InitRenderContext(m_config.backend, imguiConfig))
        return false;

    SetAutoPresent(true);

    sge::WindowSettings window_settings;
    window_settings.width = 800;
    window_settings.height = 800;
    window_settings.fullscreen = m_config.fullscreen;
    window_settings.vsync = m_config.vsync;
    window_settings.samples = m_config.samples;
    window_settings.hidden = true;

    auto result = CreateWindow(window_settings);
    if (!result.has_value()) {
        SGE_LOG_ERROR("Couldn't create a window: {}", result.error());
        return false;
    }

    std::shared_ptr<sge::GlfwWindow> window = result.value();

    LLGL::Extent2D resolution = window->GetContentSize();
    m_cameras[window->GetID()] = sge::Camera(resolution, sge::CameraConfig { .origin = sge::CameraOrigin::TopLeft });
    m_cameras[window->GetID()].set_samples(m_config.samples);

    m_renderer = std::make_unique<sge::Renderer2D>(GetRenderContext());

    m_batch = m_renderer->CreateBatch();

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    sync_time();

    window->ShowWindow();

    return true;
}

App::~App() {
    m_renderer->DestroyBatch(*m_batch);
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
        auto window = CreateWindow(sge::WindowSettings {
            .width = 500,
            .height = 500,
            .samples = 1,
            .resizable = true,
            .vsync = m_config.vsync,
        });
        if (window.has_value()) {
            const uint32_t id = window.value()->GetID();
            const LLGL::Extent2D size = window.value()->GetContentSize();
            m_cameras[id] = sge::Camera(sge::CameraConfig { .origin = sge::CameraOrigin::TopLeft });
            m_cameras[id].set_viewport(size.width, size.height);
        }
    }

    if (m_paused) return;

    m_t.time += Duration::Cast<Duration::Nanos>(Duration::SecondsFloat(Time::DeltaSeconds()));
    m_t.time = m_t.time % Duration::Hours(12);

    const float secs = static_cast<float>(m_t.time.count()) / static_cast<float>(std::nano::den);
    const float mins = secs / 60.0f;

    m_t.hours = mins / 60.0f;
    m_t.minutes = std::fmod(mins, 60.0f);
    m_t.seconds = std::fmod(secs, 60.0f);

    sge::GlfwWindow* window = sge::WindowManager::GetFocusedWindow();
    if (!window)
        return;

    sge::Camera& camera = m_cameras[window->GetID()];
    sge::Transform& camera_transform = camera.transform();

    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = camera.zoom() * zoom_factor;

        camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = camera.screen_to_world(Input::CursorPosition());
        const glm::vec2 length = mouse_pos - glm::vec2(camera_transform.translation);
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const sge::Rect& area = camera.get_projection_area();
        const glm::uvec2 window_size = camera.viewport();

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + deltaLength;
        camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size) - area.size()));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const sge::Rect& area = camera.get_projection_area();

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + Input::MouseDelta() * camera.zoom() * glm::vec2(-1.f, -1.f);
        camera.set_position(new_position);
    }

    if (Input::JustPressed(Key::Escape)) {
        camera.set_position(glm::vec2(0.0f));
        camera.set_zoom(1.0f);
    }
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
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

    sge::Camera& camera = m_cameras[window->GetID()];

    m_renderer->Begin();

    const glm::vec2 center = camera.screen_center();
    const glm::vec2 screen_size = glm::uvec2(camera.viewport());

    glm::vec2 background_size = glm::vec2(screen_size);
    float aspect = background_size.x / background_size.y;
    background_size.y *= aspect;

    const float radius = background_size.y * 0.2f;

    m_batch->DrawRect(center, {
        .size = background_size,
        .color = sge::LinearRgba(0.0f, 0.0f, 0.0f),
        .border_thickness = CLOCK_BORDER_WIDTH * background_size.x / 2.0f,
        .border_color = sge::LinearRgba(0x3B, 0x40, 0x43),
        .border_radius = sge::BorderRadius::Absolute(radius)
    });

    {
        const float padding = CLOCK_BORDER_WIDTH * background_size.x;
        glm::vec2 size = glm::vec2(screen_size - padding * 2.0f);
        const float aspect = size.x / size.y;
        size.y *= aspect;

        m_batch->DrawRect(center, {
            .size = size,
            .color = sge::LinearRgba(0x05, 0x0C, 0x0B),
            .border_radius = sge::BorderRadius::Absolute(radius - padding)
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

            m_batch->DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), sge::BorderRadius::Relative(50.0f));
        }

        for (int i = 0; i < 12; ++i) {
            float t = ((float)i) / 12.0f;
            const float sin = glm::sin(t * 2.0f * consts::PI);
            const float cos = glm::cos(t * 2.0f * consts::PI);

            if (i % 3 == 0) continue;

            const glm::vec2 dir = glm::vec2(cos, sin);

            glm::vec2 start = center - dir * (size * 0.5f - CLOCK_FACE_PADDING * size * 0.5f + (size * CLOCK_TICKS_LENGTH) * 0.2f);
            glm::vec2 line_dir = dir * (size * CLOCK_TICKS_LENGTH);

            m_batch->DrawLine(start, start + line_dir, tick_thickness, sge::LinearRgba(0xFF, 0xFF, 0xFF), sge::BorderRadius::Relative(50.0f));
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
            const float sin = glm::sin(wh - consts::FRAC_PI_2);
            const float cos = glm::cos(wh - consts::FRAC_PI_2);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const float length = hand_length - CLOCK_HOUR_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), sge::BorderRadius::Relative(50.0f));
        }

        // Minute hand
        {
            const float sin = glm::sin(wm - consts::FRAC_PI_2);
            const float cos = glm::cos(wm - consts::FRAC_PI_2);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = glm::vec2(center - line_dir * CLOCK_HAND_OFFSET * size.x);
            const float length = hand_length - CLOCK_MINUTE_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba::white(), sge::BorderRadius::Relative(50.0f));
        }

        // Second hand
        {
            const float sin = glm::sin(ws - consts::FRAC_PI_2);
            const float cos = glm::cos(ws - consts::FRAC_PI_2);
            const glm::vec2 line_dir = glm::vec2(cos, sin);

            const glm::vec2 start = center - line_dir * CLOCK_HAND_OFFSET * size.x;
            const float length = hand_length - CLOCK_SECOND_HAND_OFFSET * size.x;

            m_batch->DrawLine(start, start + line_dir * length, hand_thickness, sge::LinearRgba(0xDA, 0x30, 0x3B), sge::BorderRadius::Relative(50.0f));
        }
    }

    m_renderer->PrepareBatch(*m_batch);
    m_renderer->UploadBatchData();

    m_renderer->BeginPass(window, camera);
    {
        float red = ((float)0xC5) / 255.0f;
        float green = ((float)0xC8) / 255.0f;
        float blue = ((float)0xD3) / 255.0f;
        m_renderer->Clear(LLGL::ClearValue(red, green, blue, 1.0f));

        m_renderer->RenderBatch(*m_batch);

        #if SGE_IMGUI_ENABLED
        GetRenderContext()->BeginImGuiFrame(*window);
        {
            ImGui::NewFrame();
            {
                ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
                ImGui::ShowDemoWindow();
            }
            ImGui::Render();
        }
        GetRenderContext()->EndImGuiFrame();
        #endif
    }
    m_renderer->EndPass();

    #if SGE_IMGUI_ENABLED
    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* saved_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        for (int i = 1; i < platform_io.Viewports.Size; i++)
        {
            ImGuiViewport* viewport = platform_io.Viewports[i];
            if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
                continue;
            if (platform_io.Platform_RenderWindow) platform_io.Platform_RenderWindow(viewport, nullptr);
            if (platform_io.Renderer_RenderWindow) platform_io.Renderer_RenderWindow(viewport, nullptr);
        }
        glfwMakeContextCurrent(saved_context);
    }
    #endif

    m_renderer->End();
    m_batch->Reset();

    #if SGE_DEBUG_LAYER_ENABLED
    if (Input::Pressed(Key::C)) {
        LLGL::FrameProfile profile;
        GetRenderContext()->GetFrameProfile(&profile);
        SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
    }
    #endif
}

