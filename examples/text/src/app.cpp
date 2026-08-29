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
#include <SGE/types/color.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/utils/string.hpp>

#include <glm/trigonometric.hpp>

#include "app.hpp"
#include "SGE/renderer/batch.hpp"
#include "SGE/types/font.hpp"
#include "SGE/utils/text.hpp"

namespace Input = sge::Input;
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

    m_renderer = std::make_shared<sge::Renderer2D>(GetRenderContext());
    m_batch_vector = std::make_shared<sge::TextVectorBatch>(*m_renderer);
    m_batch_sdf = std::make_shared<sge::TextSdfBatch>(*m_renderer);

    // m_font = sge::LoadFontVector("../../examples/text/src/JetBrainsMono-Regular.ttf", *GetRenderContext());
    // m_font_sdf = sge::LoadFont("../../examples/text/src/JetBrainsMono-Regular.ttf", *GetRenderContext());

    window->ShowWindow();

    return true;
}

void App::OnUpdate() {
    ControlCamera2D(m_camera);
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    sge::ResetBatches(m_batch_vector, m_batch_sdf);

    sge::RichText text{{
        sge::RichTextSection("English:\n", sge::LinearRgba(0.3f, 0.8f, 0.3f), 96.f),
        sge::RichTextSection("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n", sge::color::WHITE, 72.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 64.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 56.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 48.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 40.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 32.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 24.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 18.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 14.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 10.f),
        sge::RichTextSection("The quick brown fox jumps over the lazy dog\n", sge::color::WHITE, 8.f),

        sge::RichTextSection("\nRussian:\n", sge::LinearRgba(0.8f, 0.3f, 0.3f), 96.f),
        sge::RichTextSection("СЪЕШЬ ЕЩЁ ЭТИХ МЯГКИХ ФРАНЦУЗСКИХ БУЛОК, ДА ВЫПЕЙ ЖЕ ЧАЮ\n", sge::color::WHITE, 72.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 64.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 56.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 48.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 40.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 32.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 24.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 18.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 14.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 10.f),
        sge::RichTextSection("Съешь ещё этих мягких французских булок, да выпей же чаю\n", sge::color::WHITE, 8.f),

        sge::RichTextSection("\nEspañol:\n", sge::LinearRgba(0.8f, 0.8f, 0.3f), 96.f),
        sge::RichTextSection("EL VELOZ MURCIÉLAGO HINDÚ COMÍA FELIZ CARDILLO Y KIWI\n", sge::color::WHITE, 72.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 64.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 56.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 48.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 40.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 32.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 24.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 18.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 14.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 10.f),
        sge::RichTextSection("El veloz murciélago hindú comía feliz cardillo y kiwi\n", sge::color::WHITE, 8.f),

        sge::RichTextSection("\nSymbols:\n", sge::LinearRgba(0.3f, 0.3f, 0.8f), 96.f),
        sge::RichTextSection("~!@#$%^&*()_-+=/,.<>\n", sge::color::WHITE, 48.f),
    }};

    const auto& font = sge::GetDefaultFontVector();
    const auto& font_sdf = sge::GetDefaultFont();

    const float text_width = sge::MeasureText(font, text).x;

    m_batch_vector->Draw(text, glm::vec2(0.0f), sge::TextAlignment::Top, font);
    m_batch_sdf->Draw(text, glm::vec2(text_width + 50.0f, 0.0f), sge::TextAlignment::Top, font_sdf);

    const float fps = 1.0 / sge::Time::DeltaSeconds();
    m_batch_vector->Draw(sge::TempFormat("FPS: {:.0f}", fps), 16.f, sge::color::WHITE, glm::vec2(15, window->GetHeight() - 30), sge::TextAlignment::Top, font, sge::Order(), sge::BatchFlags::UI);

    m_renderer->Begin();
    {
        m_renderer->BeginPass(window, m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(float(22)/0xFF, float(22)/0xFF, float(22)/0xFF, 1.f));
            m_renderer->SubmitBatches(m_batch_vector, m_batch_sdf);
            m_renderer->FlushBatches();
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

