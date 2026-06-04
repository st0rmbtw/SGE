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
#include <SGE/utils/random.hpp>

#include <glm/trigonometric.hpp>
#include <imgui.h>

#include "app.hpp"

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

namespace Input = sge::Input;
namespace Time = sge::Time;
namespace Duration = sge::Duration;
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
    m_primary_window_id = window->GetID();

    LLGL::Extent2D resolution = window->GetContentSize();
    m_camera = sge::Camera(resolution, sge::CameraConfig { .origin = sge::CameraOrigin::TopLeft });
    m_camera.set_samples(m_config.samples);

    m_renderer = std::make_unique<sge::Renderer>(GetRenderContext());

    m_batch = m_renderer->CreateBatch();

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    window->ShowWindow();

    return true;
}

App::~App() {
    m_renderer->DestroyBatch(*m_batch);
}

void App::OnUpdate() {
    if (Input::JustPressed(Key::Space)) {
        m_paused = !m_paused;
    }

    if (m_paused) return;

    sge::GlfwWindow* window = sge::WindowManager::GetFocusedWindow();
    if (!window)
        return;

    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = m_camera.zoom() * zoom_factor;

        m_camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = m_camera.screen_to_world(Input::CursorPosition());
        const glm::vec2 length = mouse_pos - m_camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const sge::Rect& area = m_camera.get_projection_area();
        const glm::uvec2 window_size = m_camera.viewport();

        const glm::vec2 new_position = m_camera.position() + deltaLength;
        m_camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size) - area.size()));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const sge::Rect& area = m_camera.get_projection_area();

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
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_camera.set_viewport(window->GetSize());

    if (m_batch_type == BatchType::Line) {
        if (m_coloring == Coloring::Random) {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 start = glm::vec2(sge::random::rand_int(0, window->GetWidth()), sge::random::rand_int(0, window->GetHeight()));
                const glm::vec2 end = start + glm::vec2(sge::random::rand_int(-500, 500), sge::random::rand_int(-500, 500));

                sge::LinearRgba color;
                color.r = sge::random::rand_float(0.0f, 1.0f);
                color.g = sge::random::rand_float(0.0f, 1.0f);
                color.b = sge::random::rand_float(0.0f, 1.0f);

                m_batch->DrawLine(start, end, 1.0f, color);
            }
        } else {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 start = glm::vec2(sge::random::rand_int(0, window->GetWidth()), sge::random::rand_int(0, window->GetHeight()));
                const glm::vec2 end = start + glm::vec2(sge::random::rand_int(-500, 500), sge::random::rand_int(-500, 500));
                m_batch->DrawLine(start, end, 1.0f, m_custom_color);
            }
        }
    } else if (m_batch_type == BatchType::Shape) {
        for (uint32_t i = 0; i < m_instance_count; ++i) {
        }
    } else if (m_batch_type == BatchType::NinePatch) {
        for (uint32_t i = 0; i < m_instance_count; ++i) {
        }
    } else if (m_batch_type == BatchType::Sprite) {
        for (uint32_t i = 0; i < m_instance_count; ++i) {
        }
    }

    m_renderer->Begin();
    {
        m_renderer->BeginPass(window, m_camera);
        {
            float red = ((float)0xC5) / 255.0f;
            float green = ((float)0xC8) / 255.0f;
            float blue = ((float)0xD3) / 255.0f;
            m_renderer->Clear(LLGL::ClearValue(red, green, blue, 1.0f));

            m_renderer->PrepareBatch(*m_batch);
            m_renderer->UploadBatchData();
            m_renderer->RenderBatch(*m_batch);

            #if SGE_IMGUI_ENABLED
            GetRenderContext()->BeginImGuiFrame(*window);
            {
                ImGui::NewFrame();
                {
                    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
                    
                    ImGui::Begin("Stress Test");
                    {
                        if (ImGui::CollapsingHeader("Settings")) {
                            ImGui::Checkbox("Paused", &m_paused);

                            static const char* batch_names[] = { "Line", "Shape", "Ninepatch", "Sprite" };
                            static int selected_batch_type = 0;

                            static const char* coloring_names[] = { "Random", "Custom" };
                            static int selected_coloring_type = 0;

                            ImGui::DragScalar("Instance Count", ImGuiDataType_U32, &m_instance_count);

                            if (ImGui::Combo("Batch Type", &selected_batch_type, batch_names, IM_ARRAYSIZE(batch_names))) {
                                m_batch_type = static_cast<BatchType>(selected_batch_type);
                            }

                            if (m_batch_type == BatchType::Line) {
                                if (ImGui::Combo("Coloring", &selected_coloring_type, coloring_names, IM_ARRAYSIZE(coloring_names))) {
                                    m_coloring = static_cast<Coloring>(selected_coloring_type);
                                }

                                if (m_coloring == Coloring::Custom) {
                                    ImGui::ColorPicker4("Custom Color", &m_custom_color.r);
                                }
                            }
                        }

                        if (ImGui::CollapsingHeader("Statistics")) {
                            ImGui::Text("Frame time: %.3f ms/frame (%.0f FPS)", Duration::GetAs<float, std::milli>(Time::Delta()), 1.0 / sge::Time::DeltaSeconds());
                        }
                    }
                    ImGui::End();
                }
                ImGui::Render();
            }
            GetRenderContext()->EndImGuiFrame();
            #endif

            m_batch->Reset();
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
    }
    m_renderer->End();
}