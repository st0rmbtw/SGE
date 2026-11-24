#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/math/consts.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/draw.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/font.hpp>
#include <SGE/types/rich_text.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/utils/string.hpp>
#include <SGE/utils/text.hpp>

#include <glm/trigonometric.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "app.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

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
    m_draw_ctx = std::make_unique<sge::BatchGroup>(m_renderer);

    // m_font = sge::LoadFontVector("../../examples/text/src/JetBrainsMono-Regular.ttf", *GetRenderContext());
    // m_font_sdf = sge::LoadFont("../../examples/text/src/JetBrainsMono-Regular.ttf", *GetRenderContext());
    //
    m_messages = std::vector<std::string>({
        "English",
        "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog",
    });
    m_sizes = std::vector<uint32_t>({
        96, 72, 64, 56, 48, 40, 32, 24, 18, 14, 10, 8
    });

    GenerateFontTextureWithFreetype(m_messages.size(), m_messages.data(), m_sizes.data());

    window->ShowWindow();

    m_scene_resolution = resolution;

    return true;
}

void App::GenerateFontTextureWithFreetype(uint32_t n, std::string* messages, uint32_t* sizes, uint32_t canvas_width, uint32_t canvas_height) {
    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    FT_New_Face(library, "../../examples/text/src/JetBrainsMono-Regular.ttf", 0, &face);

    auto atlas_data = std::make_unique<uint8_t[]>(static_cast<size_t>(canvas_width) * canvas_height * 4);

    uint32_t atlas_x = 0;
    uint32_t atlas_y = 0;

    uint32_t atlas_width = 0;

    for (uint32_t i = 0; i < n; ++i) {
        FT_Set_Pixel_Sizes(face, 0, sizes[i]);
        const uint32_t height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
        for (char c : messages[i]) {
            FT_Load_Char(face, c, m_freetype_flags);
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

            FT_Bitmap* bitmap = &face->glyph->bitmap;
            for (uint32_t y = 0; y < bitmap->rows; ++y) {
                uint8_t* row_data = bitmap->buffer + y * bitmap->pitch;
                for (uint32_t x = 0; x < bitmap->width; ++x) {
                    const size_t idx = (atlas_y + y + sizes[i] - face->glyph->bitmap_top) * canvas_width + (atlas_x + x);
                    atlas_data[idx*4 + 0] = row_data[x];
                    atlas_data[idx*4 + 1] = row_data[x];
                    atlas_data[idx*4 + 2] = row_data[x];
                    atlas_data[idx*4 + 3] = row_data[x];
                }
            }

            atlas_x += face->glyph->advance.x>>6;
            atlas_width = std::max(atlas_width, atlas_x);
        }
        atlas_x = 0;
        atlas_y += height;
    }

    atlas_width = std::max(atlas_width, 1u);
    atlas_y = std::max(atlas_y, 1u);

    sge::TextureConfig config;
    config.textureType = LLGL::TextureType::Texture2D;
    config.extent.width = atlas_width;
    config.extent.height = atlas_y;
    config.sampler = GetRenderContext()->GetNearestSampler();
    config.format = LLGL::Format::RGBA8UNorm;

    LLGL::ImageView imageView;
    imageView.data = atlas_data.get();
    imageView.dataSize = 4ULL * config.extent.width * config.extent.height;
    imageView.rowStride = canvas_width * 4ULL;
    imageView.dataType = LLGL::DataType::UInt8;
    imageView.format = LLGL::ImageFormat::RGBA;

    m_freetype_output = GetRenderContext()->CreateTexture(config, &imageView);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void App::OnInputEvent(const sge::InputEvent& event) {
#if SGE_IMGUI_ENABLED
    if (!m_simulation_view_hovered)
        return;
#endif
    Input::ProcessEvent(event);
}

void App::OnUpdate() {
    ControlCamera2D(m_camera);
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_camera.set_viewport(m_scene_resolution);

    m_draw_ctx->Reset();

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

    m_draw_ctx->Draw(sge::Draw::TextVector(text, glm::vec2(0.0f), sge::TextAlignment::Top, font));
    m_draw_ctx->Draw(sge::Draw::TextSdf(text, glm::vec2(text_width + 50.0f, 0.0f), sge::TextAlignment::Top, font_sdf));

    const float fps = 1.0 / sge::Time::DeltaSeconds();
    m_draw_ctx->Draw(
        sge::Draw::TextVector(sge::rich_text(sge::TempFormat("FPS: {:.0f}", fps), 16.f, sge::color::YELLOW), glm::vec2(15, window->GetHeight() - 30), sge::TextAlignment::Top, font)
            .WithCustomFlags(sge::BatchFlags::UI)
    );

    auto framebuffer = GetRenderContext()->GetTemporaryFramebuffer(m_scene_resolution);

    sge::TextureWithSampler framebufferTexture;
    framebufferTexture.texture = framebuffer.GetTexture();
    framebufferTexture.sampler = GetRenderContext()->GetLinearSampler()->internal();

    if (m_freetype_texture_should_regenerate) {
        GenerateFontTextureWithFreetype(m_messages.size(), m_messages.data(), m_sizes.data());
        m_freetype_texture_should_regenerate = false;
    }

    m_renderer->Begin();
    {
        m_renderer->BeginPass(*framebuffer.GetRenderTarget(), m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(float(22)/0xFF, float(22)/0xFF, float(22)/0xFF, 1.f));
            m_draw_ctx->Submit();
            m_renderer->FlushBatches();
        }
        m_renderer->EndPass();

        m_renderer->BeginPass(window, m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(0.0f, 0.0f, 0.0f, 1.f));

            #if SGE_IMGUI_ENABLED
            sge::TextureWithSampler texture;
            texture.texture = m_freetype_output.internal().Get();
            texture.sampler = m_freetype_output.sampler()->internal().Get();
            GetRenderContext()->BeginImGuiFrame(*window);
            {
                ImGui::NewFrame();
                {
                    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                   ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs;

                    ImGuiViewport* viewport = ImGui::GetMainViewport();
                    ImGui::SetNextWindowPos(viewport->WorkPos);
                    ImGui::SetNextWindowSize(viewport->WorkSize);
                    ImGui::SetNextWindowViewport(viewport->ID);

                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(float(22)/0xFF, float(22)/0xFF, float(22)/0xFF, 1.f));

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                    ImGui::Begin("RootDockSpaceWindow", nullptr, window_flags);
                    {
                        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

                        static bool layout_initialized = false;

                        if (!layout_initialized) {
                            layout_initialized = true;

                            ImGui::DockBuilderRemoveNode(dockspace_id);
                            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

                            ImGuiID dock_id_left;
                            ImGuiID dock_id_right;
                            dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.8f, nullptr, &dock_id_right);

                            ImGui::DockBuilderDockWindow("Scene", dock_id_left);
                            ImGui::DockBuilderDockWindow("FreeType Output", dock_id_right);

                            ImGuiDockNode* left_node = ImGui::DockBuilderGetNode(dock_id_left);
                            if (left_node != nullptr) {
                                left_node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
                            }

                            ImGuiDockNode* right_node = ImGui::DockBuilderGetNode(dock_id_right);
                            if (right_node != nullptr) {
                                right_node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
                            }

                            ImGui::DockBuilderFinish(dockspace_id);
                        }


                        ImGui::Begin("Scene");
                        {
                            m_simulation_view_hovered = ImGui::IsWindowHovered();

                            ImVec2 avail = ImGui::GetContentRegionAvail();
                            if (avail.x > 0.0f)
                                m_scene_resolution.width = static_cast<uint32_t>(avail.x);
                            if (avail.y > 0.0f)
                                m_scene_resolution.height = static_cast<uint32_t>(avail.y);

                            ImGui::Image((ImTextureID)&framebufferTexture, avail);
                        }
                        ImGui::End();

                        ImGui::Begin("FreeType Output");
                        {
                            ImGui::Image((ImTextureID)&texture, ImVec2(m_freetype_output.width(), m_freetype_output.height()));
                        }
                        ImGui::End();
                    }
                    ImGui::End();
                    ImGui::PopStyleVar(3);

                    ImGui::Begin("FreeType Settings");
                    {
                        for (size_t i = 0; i < m_messages.size(); ++i) {
                            ImGui::Text("%zu. %s at %u px", i+1, m_messages[i].c_str(), m_sizes[i]);
                        }

                        static std::string inputBuffer = {};
                        static uint32_t inputSize = 0;
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
                        ImGui::InputText("##MessageInput", &inputBuffer);
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.05f);
                        ImGui::InputScalar("##Size", ImGuiDataType_U32, &inputSize);
                        ImGui::PopItemWidth();
                        ImGui::SameLine();

                        const bool isValid = inputSize > 0 && !inputBuffer.empty();

                        if (!isValid)
                            ImGui::BeginDisabled();

                        if (ImGui::Button("+")) {
                            m_messages.push_back(inputBuffer);
                            m_sizes.push_back(inputSize);
                            inputBuffer.clear();
                        }

                        if (!isValid)
                            ImGui::EndDisabled();

                        if (ImGui::Button("Clear Messages")) {
                            m_messages.clear();
                            m_sizes.clear();
                        }

                        if (ImGui::CollapsingHeader("FreeType Load Flags")) {
                            ImGui::CheckboxFlags("FT_LOAD_NO_SCALE", &m_freetype_flags, FT_LOAD_NO_SCALE);
                            ImGui::CheckboxFlags("FT_LOAD_NO_HINTING", &m_freetype_flags, FT_LOAD_NO_HINTING);
                            ImGui::CheckboxFlags("FT_LOAD_RENDER", &m_freetype_flags, FT_LOAD_RENDER);
                            ImGui::CheckboxFlags("FT_LOAD_NO_BITMAP", &m_freetype_flags, FT_LOAD_NO_BITMAP);
                            ImGui::CheckboxFlags("FT_LOAD_VERTICAL_LAYOUT", &m_freetype_flags, FT_LOAD_VERTICAL_LAYOUT);
                            ImGui::CheckboxFlags("FT_LOAD_FORCE_AUTOHINT", &m_freetype_flags, FT_LOAD_FORCE_AUTOHINT);
                            ImGui::CheckboxFlags("FT_LOAD_CROP_BITMAP", &m_freetype_flags, FT_LOAD_CROP_BITMAP);
                            ImGui::CheckboxFlags("FT_LOAD_PEDANTIC", &m_freetype_flags, FT_LOAD_PEDANTIC);
                            ImGui::CheckboxFlags("FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH", &m_freetype_flags, FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
                            ImGui::CheckboxFlags("FT_LOAD_NO_RECURSE", &m_freetype_flags, FT_LOAD_NO_RECURSE);
                            ImGui::CheckboxFlags("FT_LOAD_IGNORE_TRANSFORM", &m_freetype_flags, FT_LOAD_IGNORE_TRANSFORM);
                            ImGui::CheckboxFlags("FT_LOAD_MONOCHROME", &m_freetype_flags, FT_LOAD_MONOCHROME);
                            ImGui::CheckboxFlags("FT_LOAD_LINEAR_DESIGN", &m_freetype_flags, FT_LOAD_LINEAR_DESIGN);
                            ImGui::CheckboxFlags("FT_LOAD_SBITS_ONLY", &m_freetype_flags, FT_LOAD_SBITS_ONLY);
                            ImGui::CheckboxFlags("FT_LOAD_NO_AUTOHINT", &m_freetype_flags, FT_LOAD_NO_AUTOHINT);
                        }

                        ImGui::Separator();

                        if (ImGui::Button("Generate texture", ImVec2(-1.f, 0.f))) {
                            m_freetype_texture_should_regenerate = true;
                        }
                    }
                    ImGui::End();

                    ImGui::PopStyleColor();
                }
                ImGui::Render();
            }
            #endif
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
