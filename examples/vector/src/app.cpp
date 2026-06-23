#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/math/consts.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/window_settings.hpp>

#include <glm/trigonometric.hpp>

#include "app.hpp"

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

namespace Input = sge::Input;
namespace Time = sge::Time;
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
    m_camera = sge::Camera(resolution, sge::CameraConfig { .origin = sge::CameraOrigin::Center });
    m_camera.set_samples(m_config.samples);

    m_renderer = std::make_unique<sge::Renderer2D>(GetRenderContext());

    m_path.SetScale(glm::vec2(350.0f));

    m_path.Begin(glm::vec2(1.24264, 8.24264) / 16.0f);
    m_path.LineTo(glm::vec2(8, 15) / 16.0f);
    m_path.LineTo(glm::vec2(14.7574, 8.24264) / 16.0f);
    m_path.CubicBezierTo(glm::vec2(15.553, 7.44699) / 16.0f, glm::vec2(16, 6.36786) / 16.0f, glm::vec2(16, 5.24264) / 16.0f);
    m_path.VerticalTo(5.05234 / 16.0f);
    m_path.CubicBezierTo(glm::vec2(16, 2.8143) / 16.0f, glm::vec2(14.1857, 1)  / 16.0f, glm::vec2(11.9477, 1) / 16.0f);
    m_path.CubicBezierTo(glm::vec2(10.7166, 1) / 16.0f, glm::vec2(9.55233, 1.55959) / 16.0f, glm::vec2(8.78331, 2.52086) / 16.0f);
    m_path.LineTo(glm::vec2(8, 3.5) / 16.0f);
    m_path.LineTo(glm::vec2(7.21669, 2.52086) / 16.0f);
    m_path.CubicBezierTo(glm::vec2(6.44767, 1.55959)  / 16.0f, glm::vec2(5.28338, 1) / 16.0f, glm::vec2(4.05234, 1) / 16.0f);
    m_path.CubicBezierTo(glm::vec2(1.8143, 1) / 16.0f, glm::vec2(0, 2.8143) / 16.0f, glm::vec2(0, 5.05234) / 16.0f);
    m_path.VerticalTo(5.24264 / 16.0f);
    m_path.CubicBezierTo(glm::vec2(0, 6.36786) / 16.0f, glm::vec2(0.44699, 7.44699) / 16.0f, glm::vec2(1.24264, 8.24264) / 16.0f);
    m_path.End();

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    window->ShowWindow();

    return true;
}

App::~App() {
}

void App::OnUpdate() {
    sge::Transform& camera_transform = m_camera.transform();

    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = m_camera.zoom() * zoom_factor;

        m_camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = m_camera.screen_to_world(Input::CursorPosition());
        const glm::vec2 length = mouse_pos - glm::vec2(camera_transform.translation);
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const sge::Rect& area = m_camera.get_projection_area();
        const glm::vec2 window_size = glm::vec2(m_camera.viewport());

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + deltaLength;
        m_camera.set_position(glm::clamp(new_position, area.size() - window_size, window_size - area.size()));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const sge::Rect& area = m_camera.get_projection_area();

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + Input::MouseDelta() * m_camera.zoom() * glm::vec2(-1.f, -1.f);
        m_camera.set_position(new_position);
    }
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_renderer->Begin();
    {
        auto framebuffer = GetRenderContext()->GetTemporaryFramebuffer(window->GetSize(), LLGL::Format::RG11B10Float);
        
        m_renderer->BeginPass(*framebuffer.GetRenderTarget(), m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(0.6f, 0.3f, 0.3f, 1.0f));
            m_renderer->DrawPath(m_path, sge::LinearRgba(5.0f, 0.4f, 0.4f), sge::Transform::FromTranslation(glm::vec3(-m_path.GetBounds().size() * 0.5f, 0.0f)));
        }
        m_renderer->EndPass();
        
        m_renderer->BloomPass(framebuffer);
        m_renderer->TonemapPass(framebuffer);

        m_renderer->BeginPass(window);
        {
            m_renderer->BlitTexture(*framebuffer.GetTexture());
        }
        m_renderer->EndPass();
    }
    m_renderer->End();

    #if SGE_DEBUG
    if (Input::Pressed(Key::C)) {
        LLGL::FrameProfile profile;
        GetRenderContext()->GetDebugInfo(&profile);
        SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
    }
    #endif
}

