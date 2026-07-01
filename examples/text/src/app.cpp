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
#include <SGE/types/rich_text.hpp>
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
    m_camera = sge::Camera(resolution, sge::CameraConfig { .origin = sge::CameraOrigin::TopLeft });
    m_camera.set_samples(m_config.samples);

    m_renderer = std::make_unique<sge::Renderer2D>(GetRenderContext());
    
    m_batch = m_renderer->CreateBatch();

    m_font = sge::LoadFontVector("../../examples/text/src/JetBrainsMono-Regular.ttf", *GetRenderContext());

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    window->ShowWindow();

    return true;
}

App::~App() {
}

void App::OnUpdate() {
    sge::Transform& camera_transform = m_camera.transform();

    for (const float scroll : Input::ScrollEvents()) {
        const float new_zoom = glm::clamp(m_camera.zoom() * glm::pow(0.75f, scroll), 0.f, 10.f);
        const float zoom_delta = new_zoom / m_camera.zoom();

        m_camera.set_zoom(new_zoom);

        const glm::vec2 mouse_pos = m_camera.screen_to_world(Input::CursorPosition());
        const glm::vec2 length = mouse_pos - glm::vec2(camera_transform.translation);
        const glm::vec2 scaled_length = length * zoom_delta;
        const glm::vec2 delta_length = length - scaled_length;

        const sge::Rect& area = m_camera.get_projection_area();
        const glm::vec2 window_size = glm::vec2(m_camera.viewport());

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + delta_length;
        m_camera.set_position(new_position);
    }

    if (Input::Pressed(MouseButton::Left)) {
        const sge::Rect& area = m_camera.get_projection_area();

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + Input::MouseDelta() * m_camera.zoom() * glm::vec2(-1.f, -1.f);
        m_camera.set_position(new_position);
    }

    if (Input::Pressed(Key::Escape)) {
        m_camera.set_zoom(1.f);
        m_camera.set_position(glm::vec2(0.f));
    }
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_batch->Reset();
    sge::RichText text{{
        sge::RichTextSection("English:\n", sge::LinearRgba(0.3f, 0.8f, 0.3f), 96.f),
        sge::RichTextSection("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n", sge::LinearRgba::white(), 72.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 64.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 56.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 48.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 40.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 32.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 24.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 18.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 14.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 10.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::LinearRgba::white(), 8.f),

        sge::RichTextSection("\nRussian:\n", sge::LinearRgba(0.8f, 0.3f, 0.3f), 96.f),
        sge::RichTextSection("СЪЕШЬ ЕЩЁ ЭТИХ МЯГКИХ ФРАНЦУЗСКИХ БУЛОК, ДА ВЫПЕЙ ЖЕ ЧАЮ\n", sge::LinearRgba::white(), 72.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 64.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 56.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 48.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 40.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 32.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 24.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 18.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 14.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 10.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::LinearRgba::white(), 8.f),
        
        sge::RichTextSection("\nEspañol:\n", sge::LinearRgba(0.8f, 0.8f, 0.3f), 96.f),
        sge::RichTextSection("EL VELOZ MURCIÉLAGO HINDÚ COMÍA FELIZ CARDILLO Y KIWI\n", sge::LinearRgba::white(), 72.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 64.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 56.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 48.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 40.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 32.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 24.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 18.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 14.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 10.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::LinearRgba::white(), 8.f),

        sge::RichTextSection("\nSymbols:\n", sge::LinearRgba(0.3f, 0.3f, 0.8f), 96.f),
        sge::RichTextSection("~!@#$%^&*()_-+=/,.<>\n", sge::LinearRgba::white(), 48.f),
    }};

    m_batch->DrawTextVector(text, glm::vec2(0.0f), m_font);

    m_renderer->Begin();
    {
        m_renderer->PrepareBatch(*m_batch);
        m_renderer->UploadBatchData();

        m_renderer->BeginPass(window, m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(float(22)/0xFF, float(22)/0xFF, float(22)/0xFF, 1.f));
            m_renderer->RenderBatch(*m_batch);
        }
        m_renderer->EndPass();
    }
    m_renderer->End();

    #if SGE_DEBUG_LAYER_ENABLED
    if (Input::Pressed(Key::C)) {
        LLGL::FrameProfile profile;
        GetRenderContext()->GetFrameProfile(&profile);
        SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
    }
    #endif
}

