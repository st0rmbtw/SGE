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
#include <SGE/types/path_builder.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/transform.hpp>
#include <SGE/types/vector_shape.hpp>
#include <SGE/types/window_settings.hpp>

#include <glm/trigonometric.hpp>

#include "app.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <misc/cpp/imgui_stdlib.h>

namespace Input = sge::Input;
namespace Time = sge::Time;
using Key = sge::Key;
using MouseButton = sge::MouseButton;

namespace {

bool ParseSvgPath(sge::PathBuilder& builder, std::string_view svg) {
    size_t cursor = 0;

    std::vector<float> argumentStack;
    char currentMode = 0;

    do {
        switch (currentMode) {
        case 'M': {
            if (argumentStack.size() == 2) {
                auto y = argumentStack.back(); argumentStack.pop_back();
                auto x = argumentStack.back(); argumentStack.pop_back();
                builder.MoveTo(glm::vec2(x, y));
            }
            break;
        }
        case 'L': {
            if (argumentStack.size() == 2) {
                auto dy = argumentStack.back(); argumentStack.pop_back();
                auto dx = argumentStack.back(); argumentStack.pop_back();
                builder.LineTo(glm::vec2(dx, dy));
            }
            break;
        }
        case 'C': {
            if (argumentStack.size() == 6) {
                auto dy3 = argumentStack.back(); argumentStack.pop_back();
                auto dx3 = argumentStack.back(); argumentStack.pop_back();
                auto dy2 = argumentStack.back(); argumentStack.pop_back();
                auto dx2 = argumentStack.back(); argumentStack.pop_back();
                auto dy1 = argumentStack.back(); argumentStack.pop_back();
                auto dx1 = argumentStack.back(); argumentStack.pop_back();
                builder.CubicBezierTo({dx1, dy1}, {dx2, dy2}, {dx3, dy3});
            }
            break;
        }
        case 'h': {
            if (argumentStack.size() == 1) {
                auto dx = argumentStack.back(); argumentStack.pop_back();
                builder.HorizontalToRel(dx);
            }
            break;
        }
        case 'v': {
            if (argumentStack.size() == 1) {
                auto dy = argumentStack.back(); argumentStack.pop_back();
                builder.VerticalToRel(dy);
            }
            break;
        }
        case 'l': {
            if (argumentStack.size() == 2) {
                auto dy = argumentStack.back(); argumentStack.pop_back();
                auto dx = argumentStack.back(); argumentStack.pop_back();
                builder.LineToRel(glm::vec2(dx, dy));
            }
            break;
        }
        case 'c': {
            if (argumentStack.size() == 6) {
                auto dy3 = argumentStack.back(); argumentStack.pop_back();
                auto dx3 = argumentStack.back(); argumentStack.pop_back();
                auto dy2 = argumentStack.back(); argumentStack.pop_back();
                auto dx2 = argumentStack.back(); argumentStack.pop_back();
                auto dy1 = argumentStack.back(); argumentStack.pop_back();
                auto dx1 = argumentStack.back(); argumentStack.pop_back();
                builder.CubicBezierToRel({dx1, dy1}, {dx2, dy2}, {dx3, dy3});
            }
            break;
        }
        case 'z':
        case 'Z': {
            builder.Close();
            currentMode = 0;
            break;
        }
        case 0: break;
        default:
            SGE_PANIC("unknown command: '{}'", currentMode);
        }

        if (cursor >= svg.size()) break;

        switch (svg[cursor]) {
        case ',':
        case ' ':
            ++cursor;
            continue;
        default:
        }

        if (std::isalpha(svg[cursor])) {
            currentMode = svg[cursor];
            ++cursor;
        }

        auto begin = svg.data() + cursor;
        char* end = nullptr;
        double arg = std::strtod(begin, &end);
        if (begin != end) {
            argumentStack.push_back(arg);
            cursor += end - begin;
        }
    } while (true);

    return true;
}

} // namespace

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

    m_imgui_state.scene_resolution = resolution;

    m_renderer = std::make_shared<sge::Renderer2D>(GetRenderContext());
    m_batch_group = std::make_unique<sge::BatchGroup>(m_renderer);

    ParseSvgPath(m_path_builder, "M59,52c-2,0,0,1,2,1l2-1h-4M67,50l2-4l-1,3c-3,2,0-1,0-2c-3,4-1,2-1,3M70,42c1-3,0-2,0-1v1M53,3l2,1c1,0,1-1-2-1M55,4h-1zM81,43c0,3-1,4-2,6l-1,1l-1,3l-6,5c-1,0,0-1,1-1c-3,1-2,2-6,3c-9,4-21-4-21-15c-1-5,2-11,8-13c5-3,11-2,14,1c-2-2-6-5-10-5c-5,0-9,3-11,7c-2,1-2,5-3,6c-2,10,2,15,9,20c1,1,0,1,0,1l-6-4l4,3c-3,0-5-5-6-5c3,7,15,12,21,9c-3,0-7,0-10-1c-1-1-3-2-2-2c8,3,16,2,23-4l4-3c-1,1,0,0,0,1c1-2-1-1,1-4l1,1c0-3,3-5,3-9c1-1,1,1,0,4c1-3,0-4,1-6c0,1,0,2,1,2c-1-2,0-4,1-6c-1,0-1,1-2-2c0-1,1-1,1-1l-2-3c1-1,1,1,2,1l-1-4c-1-3-1,0-2-2c-1-4,2-1,2-3l4,10l-2-7c0,1-1-5,1-1c-2-8-9-14-15-18c1,1,2,2,1,2c-3-2-2-2-3-3c-2-1-2,1-4,0c-4-2-5-2-9-3v1c-3-1-4,0-7,0c0-1,1-1,2-1c-3,0-3-1-6,0l3-1c-3,0-6,1-5,0c-4,2-11,4-15,8v-1c-2,2-8,6-8,9h-1c-1,2-1,4-2,5c-1,2-2,1-2,1l-4,13c1,1,0,6,0,11c-1,21,15,42,33,47c3,1,7,1,10,1l-8-2c-3-1-3-3-5-4v1c-3-1-2-2-5-3l1-1c-1,0-3-2-4-3h-1l-3-4c-1-1-6-7-3-6c-1,0-2,0-2-2l-2-5c0,1,1,1,2,1c-4-9-4,0-7-9h1c-1,0-1-1-1-2v-2c-3-3-1-12,0-18c0-2,2-4,3-7h-1c2-3,10-12,13-11c2-3,0,0-1-1c4-4,5-3,8-3c3-2-2,0-1-1c5-1,3-3,9-3c1,0-1,0-2,1c4-2,13-2,18,1c7,3,14,12,14,20h1c0,3,0,7-1,11zM42,54v2l3,4l-3-6M44,54l-1-2c0,2,1,3,2,4zM87,45v1c0,3-1,5-3,8c2-3,3-6,3-9M53,3c1-1,3-1,4-1h-4zM16,22c0,3-2,4,0,2c2-3,0-1,0-2M13,33l1-3c-1,1,0,2-1,3");

    sge::VectorShapeLayer layer{
        .path = m_path_builder.Build(),
        .customSize = glm::vec2{ 100.f, 100.f }
    };
    m_vector_shape = GetRenderContext()->CreateVectorShape({ std::views::single(layer) });

    window->ShowWindow();

    return true;
}

App::~App() {
}

void App::OnUpdate() {
    ControlCamera2D(m_camera);
}

void App::OnInputEvent(const sge::InputEvent& event) {
#if SGE_IMGUI_ENABLED
    if (!m_imgui_state.scene_hovered)
        return;
#endif
    Input::ProcessEvent(event);
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    m_camera.set_viewport(m_imgui_state.scene_resolution);

    m_batch_group->Reset();
    m_batch_group->Draw(sge::VectorShapeDrawData {
        .shape = m_vector_shape,
        .position = m_camera.screen_center(),
        .size = glm::vec2{ 200.f },
    });

    m_renderer->Begin();
    {
        sge::TemporaryFramebufferOptions tempFramebufferOpts;
        tempFramebufferOpts.colorFormat = LLGL::Format::RG11B10Float;
        tempFramebufferOpts.samples = window->GetSamples();
        tempFramebufferOpts.bindFlags = LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled | LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst;
        auto framebuffer = GetRenderContext()->GetTemporaryFramebuffer(window->GetSize(), tempFramebufferOpts);

        sge::TextureWithSampler framebufferTexture;
        framebufferTexture.texture = framebuffer.GetTexture();
        framebufferTexture.sampler = GetRenderContext()->GetLinearSampler()->internal().Get();

        m_batch_group->Submit();
        m_renderer->FlushBatches();

        m_renderer->BeginPass(*framebuffer.GetRenderTarget(), m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(0.1f, 0.1f, 0.1f, 1.0f));
        }
        m_renderer->EndPass();

        m_renderer->BloomPass(framebuffer);
        m_renderer->TonemapPass(framebuffer);

        m_renderer->BeginPass(window);
        {
            #if !SGE_IMGUI_ENABLED
            m_renderer->BlitTexture(*framebuffer.GetTexture());
            #else
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
                            ImGui::DockBuilderDockWindow("Settings", dock_id_right);

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
                            m_imgui_state.scene_hovered = ImGui::IsWindowHovered();

                            ImVec2 avail = ImGui::GetContentRegionAvail();
                            if (avail.x > 0.0f)
                                m_imgui_state.scene_resolution.width = static_cast<uint32_t>(avail.x);
                            if (avail.y > 0.0f)
                                m_imgui_state.scene_resolution.height = static_cast<uint32_t>(avail.y);

                            ImGui::Image((ImTextureID)&framebufferTexture, avail);
                        }
                        ImGui::End();
                    }
                    ImGui::End();
                    ImGui::PopStyleVar(3);

                    ImGui::Begin("Settings");
                    {
                        ImGui::PushItemWidth(-1);
                        static std::string inputBuffer = {};
                        ImGui::InputText("##PathInput", &inputBuffer);
                        ImGui::PopItemWidth();

                        static float shapeSizeX = 0.0f;
                        static float shapeSizeY = 0.0f;
                        ImGui::InputFloat("##SizeX", &shapeSizeX);
                        ImGui::SameLine();
                        ImGui::InputFloat("##SizeY", &shapeSizeY);

                        ImGui::PushItemWidth(-1);
                        ImGui::BeginDisabled(inputBuffer.empty());
                        if (ImGui::Button("Generate path")) {
                            m_path_builder.Reset();
                            ParseSvgPath(m_path_builder, inputBuffer);

                            std::optional<glm::vec2> customSize;
                            if (shapeSizeX > 0.0 && shapeSizeY > 0.0)
                                customSize = glm::vec2(shapeSizeX, shapeSizeY);

                            sge::VectorShapeLayer layer{
                                .path = m_path_builder.Build(),
                                .customSize = customSize
                            };
                            m_vector_shape = GetRenderContext()->CreateVectorShape({ std::views::single(layer) });
                        }
                        ImGui::EndDisabled();
                        ImGui::PopItemWidth();

                        if (ImGui::CollapsingHeader("Current Shape Info")) {
                            const auto& layers = m_vector_shape->GetLayers();
                            for (size_t i = 0; i < layers.size(); ++i) {
                                const auto& l = layers[i];
                                ImGui::Text("Layer %zu:", i);
                                ImGui::Text("- Band Height: %.2f", l.bandHeight);
                                ImGui::Text("- Size: (%.2f, %.2f)", l.size.x, l.size.y);
                                ImGui::Text("- Partition Count: %u", l.partitionCount);
                            }
                        }

                        DisplayFrameStatistics(*GetRenderContext(), m_profile);
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
