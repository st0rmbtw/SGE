#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/math/consts.hpp>
#include <SGE/profile.hpp>
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

    m_renderer = std::make_unique<sge::Renderer2D>(GetRenderContext());

    m_batch = m_renderer->CreateBatch();

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    window->ShowWindow();

    return true;
}

App::~App() {
    m_renderer->DestroyBatch(*m_batch);
}

void App::DrawContent(LLGL::Extent2D viewport) {
    ZoneScoped;

    if (m_batch_type == BatchType::Line) {
        if (m_coloring == Coloring::Random) {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 size = sge::Random::UVec2(m_size_from, m_size_to);
                const glm::vec2 start = sge::Random::UVec2(glm::uvec2(0), glm::uvec2(viewport.width, viewport.height));
                const glm::vec2 end = start + size;

                sge::LinearRgba color;
                color.r = sge::Random::Float(0.0f, 1.0f);
                color.g = sge::Random::Float(0.0f, 1.0f);
                color.b = sge::Random::Float(0.0f, 1.0f);

                m_batch->DrawLine(start, end, 1.0f, color);
            }
        } else {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 size = sge::Random::UVec2(m_size_from, m_size_to);
                const glm::vec2 start = sge::Random::UVec2(glm::uvec2(0), glm::uvec2(viewport.width, viewport.height));
                const glm::vec2 end = start + size;
                m_batch->DrawLine(start, end, 1.0f, m_custom_color);
            }
        }
    } else if (m_batch_type == BatchType::Shape) {
        if (m_coloring == Coloring::Random) {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 position = sge::Random::UVec2(glm::uvec2(0), glm::uvec2(viewport.width, viewport.height));
                
                sge::LinearRgba color;
                color.r = sge::Random::Float(0.0f, 1.0f);
                color.g = sge::Random::Float(0.0f, 1.0f);
                color.b = sge::Random::Float(0.0f, 1.0f);
                
                if (m_shape_type == sge::Shape::Rect) {
                    const glm::vec2 size = sge::Random::UVec2(m_size_from, m_size_to);

                    m_batch->DrawRect(position, sge::ShapeRect {
                        .size = size,
                        .color = color
                    });
                } else if (m_shape_type == sge::Shape::Circle) {
                    const float radius = sge::Random::Float(m_radius_from, m_radius_to);

                    m_batch->DrawCircle(position, sge::ShapeCircle {
                        .radius = radius,
                        .color = color
                    });
                } else if (m_shape_type == sge::Shape::Arc) {
                    const float outer_radius = sge::Random::Float(m_outer_radius_from, m_outer_radius_to);
                    const float inner_radius = sge::Random::Float(m_inner_radius_from, m_inner_radius_to);
                    const float start_angle = sge::Random::Float(-sge::consts::PI, sge::consts::PI);
                    const float end_angle = sge::Random::Float(-sge::consts::PI, sge::consts::PI);

                    m_batch->DrawArc(position, sge::ShapeArc {
                        .outer_radius = outer_radius,
                        .inner_radius = inner_radius,
                        .start_angle = start_angle,
                        .end_angle = end_angle,
                        .color = color
                    });
                }
            }
        } else {
            for (uint32_t i = 0; i < m_instance_count; ++i) {
                const glm::vec2 position = sge::Random::UVec2(glm::uvec2(0), glm::uvec2(viewport.width, viewport.height));
                
                if (m_shape_type == sge::Shape::Rect) {
                    const glm::vec2 size = sge::Random::UVec2(m_size_from, m_size_to);

                    m_batch->DrawRect(position, sge::ShapeRect {
                        .size = size,
                        .color = m_custom_color
                    });
                } else if (m_shape_type == sge::Shape::Circle) {
                    const float radius = sge::Random::Float(m_radius_from, m_radius_to);

                    m_batch->DrawCircle(position, sge::ShapeCircle {
                        .radius = radius,
                        .color = m_custom_color
                    });
                } else if (m_shape_type == sge::Shape::Arc) {
                    const float outer_radius = sge::Random::Float(m_outer_radius_from, m_outer_radius_to);
                    const float inner_radius = sge::Random::Float(m_inner_radius_from, m_inner_radius_to);
                    const float start_angle = sge::Random::Float(-sge::consts::PI, sge::consts::PI);
                    const float end_angle = sge::Random::Float(-sge::consts::PI, sge::consts::PI);

                    m_batch->DrawArc(position, sge::ShapeArc {
                        .outer_radius = outer_radius,
                        .inner_radius = inner_radius,
                        .start_angle = start_angle,
                        .end_angle = end_angle,
                        .color = m_custom_color
                    });
                }
            }
        }
    } else if (m_batch_type == BatchType::NinePatch) {
        // TODO
    } else if (m_batch_type == BatchType::Sprite) {
        // TODO
    }
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_camera.set_viewport(window->GetSize());

    m_batch->BeginOrderMode();
    {
        DrawContent(window->GetSize());
    }
    m_batch->EndOrderMode();

    m_renderer->Begin();
    {
        m_renderer->PrepareBatch(*m_batch);
        m_renderer->UploadBatchData();

        m_renderer->BeginPass(window, m_camera);
        {
            static constexpr float red = ((float)0xC5) / 255.0f;
            static constexpr float green = ((float)0xC8) / 255.0f;
            static constexpr float blue = ((float)0xD3) / 255.0f;
            m_renderer->Clear(LLGL::ClearValue(red, green, blue, 1.0f));

            m_renderer->RenderBatch(*m_batch);

            GetRenderContext()->BeginImGuiFrame(*window);
            {
                ImGui::NewFrame();
                {
                    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
                    
                    ImGui::Begin("Stress Test");
                    {
                        ImGui::Text("Renderer: %s", GetRenderContext()->GetRendererInfo().rendererName.c_str());

                        if (ImGui::CollapsingHeader("Settings")) {
                            if (ImGui::DragScalar("Batch Limit", ImGuiDataType_U32, &m_batch_limit, 100.0f)) {
                                m_batch->SetMaxCount((m_batch_limit == 0) ? UINT32_MAX : m_batch_limit);
                            }
                            ImGui::DragScalar("Instance Count", ImGuiDataType_U32, &m_instance_count);
                            
                            static const char* batch_names[] = { "Line", "Shape", "Ninepatch", "Sprite" };
                            static int selected_batch_type = 0;
                            if (ImGui::Combo("Batch Type", &selected_batch_type, batch_names, IM_ARRAYSIZE(batch_names))) {
                                m_batch_type = static_cast<BatchType>(selected_batch_type);
                            }

                            if (m_batch_type == BatchType::Shape) {
                                static const char* shape_names[] = { "Rect", "Circle", "Arc" };
                                static int selected_shape_type = 0;
                                if (ImGui::Combo("Shape Type", &selected_shape_type, shape_names, IM_ARRAYSIZE(shape_names))) {
                                    m_shape_type = static_cast<sge::Shape::Type>(selected_shape_type);
                                }

                                if (m_shape_type == sge::Shape::Rect) {
                                    ImGui::SeparatorText("Size From");
                                    ImGui::DragScalar("X##1", ImGuiDataType_U32, &m_size_from.x);
                                    ImGui::DragScalar("Y##1", ImGuiDataType_U32, &m_size_from.y);

                                    ImGui::SeparatorText("Size To");
                                    ImGui::DragScalar("X##2", ImGuiDataType_U32, &m_size_to.x);
                                    ImGui::DragScalar("Y##2", ImGuiDataType_U32, &m_size_to.y);
                                } else if (m_shape_type == sge::Shape::Circle) {
                                    ImGui::SeparatorText("Radius");
                                    ImGui::DragFloat("From##Radius", &m_radius_from, 1.0f, 0.0f, FLT_MAX);
                                    ImGui::DragFloat("To##Radius", &m_radius_to, 1.0f, 0.0f, FLT_MAX);
                                } else if (m_shape_type == sge::Shape::Arc) {
                                    ImGui::SeparatorText("Inner Radius");
                                    ImGui::DragFloat("From##InnerRadius", &m_inner_radius_from, 1.0f, 0.0f, FLT_MAX);
                                    ImGui::DragFloat("To##InnerRadius", &m_inner_radius_to, 1.0f, 0.0f, FLT_MAX);
                                    ImGui::SeparatorText("Outer Radius");
                                    ImGui::DragFloat("From##OuterRadius", &m_outer_radius_from, 1.0f, 0.0f, FLT_MAX);
                                    ImGui::DragFloat("To##OuterRadius", &m_outer_radius_to, 1.0f, 0.0f, FLT_MAX);
                                }
                            }

                            if (m_batch_type == BatchType::Line) {
                                ImGui::SeparatorText("Size From");
                                ImGui::DragScalar("X##1", ImGuiDataType_U32, &m_size_from.x);
                                ImGui::DragScalar("Y##1", ImGuiDataType_U32, &m_size_from.y);

                                ImGui::SeparatorText("Size To");
                                ImGui::DragScalar("X##2", ImGuiDataType_U32, &m_size_to.x);
                                ImGui::DragScalar("Y##2", ImGuiDataType_U32, &m_size_to.y);
                            }

                            static const char* property_names[] = { "Random", "Custom" };
                            static int selected_coloring_type = 0;
                            if (ImGui::Combo("Coloring", &selected_coloring_type, property_names, IM_ARRAYSIZE(property_names))) {
                                m_coloring = static_cast<Coloring>(selected_coloring_type);
                            }
                            if (m_coloring == Coloring::Custom) {
                                ImGui::ColorPicker4("Custom Color", &m_custom_color.r);
                            }
                        }

                        if (ImGui::CollapsingHeader("Statistics")) {
                            ImGui::Text("Frame time: %.3f ms/frame (%.0f FPS)", Duration::GetAs<float, std::milli>(Time::Delta()), 1.0 / sge::Time::DeltaSeconds());
                            
                            #if SGE_DEBUG
                            LLGL::FrameProfile profile;
                            GetRenderContext()->GetDebugInfo(&profile);

                            ImGui::Text("Draw commands: %d", profile.commandBufferRecord.drawCommands);
                            #endif
                        }
                    }
                    ImGui::End();
                }
                ImGui::Render();
            }
            GetRenderContext()->EndImGuiFrame();
        }
        m_renderer->EndPass();

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
    }
    m_renderer->End();
    m_batch->Reset();
}