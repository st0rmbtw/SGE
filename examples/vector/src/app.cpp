#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/log.hpp>
#include <SGE/math/consts.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/context.hpp>
#include <SGE/renderer/glfw_window.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/time/stopwatch.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/color.hpp>
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

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

namespace Input = sge::Input;
namespace Time = sge::Time;
using Key = sge::Key;
using MouseButton = sge::MouseButton;

namespace {

bool ParseSvg(std::string_view svg, sge::PathBuilder& builder) {
    size_t cursor = 0;

    std::vector<float> argumentStack;
    char currentMode = 0;

    do {
        if (currentMode == 'M' && argumentStack.size() == 2) {
            auto y = argumentStack.back(); argumentStack.pop_back();
            auto x = argumentStack.back(); argumentStack.pop_back();
            builder.MoveTo(glm::vec2(x, y));
            currentMode = 0;
        }

        if (currentMode == 'h' && argumentStack.size() == 1) {
            auto dx = argumentStack.back(); argumentStack.pop_back();
            builder.HorizontalToRel(dx);
            currentMode = 0;
        }

        if (currentMode == 'v' && argumentStack.size() == 1) {
            auto dy = argumentStack.back(); argumentStack.pop_back();
            builder.VerticalToRel(dy);
            currentMode = 0;
        }

        if (currentMode == 'l' && argumentStack.size() == 2) {
            auto dy = argumentStack.back(); argumentStack.pop_back();
            auto dx = argumentStack.back(); argumentStack.pop_back();
            builder.LineToRel(glm::vec2(dx, dy));
            currentMode = 0;
        }

        if (currentMode == 'c' && argumentStack.size() == 6) {
            auto dy3 = argumentStack.back(); argumentStack.pop_back();
            auto dx3 = argumentStack.back(); argumentStack.pop_back();
            auto dy2 = argumentStack.back(); argumentStack.pop_back();
            auto dx2 = argumentStack.back(); argumentStack.pop_back();
            auto dy1 = argumentStack.back(); argumentStack.pop_back();
            auto dx1 = argumentStack.back(); argumentStack.pop_back();
            builder.CubicBezierToRel({dx1, dy1}, {dx2, dy2}, {dx3, dy3});
            currentMode = 0;
        }

        if (currentMode == 'Z' || currentMode == 'z') {
            builder.Close();
            currentMode = 0;
        }

        if (cursor >= svg.size()) break;

        switch (svg[cursor]) {
        case ',':
        case ' ':
            ++cursor;
            continue;
        default:
        }

        if (currentMode == 0) {
            SGE_ASSERT(std::isalpha(svg[cursor]));
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

    m_path = m_path_builder
        .MoveTo(glm::vec2(1.24264, 8.24264))
        .LineTo(glm::vec2(8, 15))
        .LineTo(glm::vec2(14.7574, 8.24264))
        .CubicBezierTo(glm::vec2(15.553, 7.44699), glm::vec2(16, 6.36786), glm::vec2(16, 5.24264))
        .VerticalTo(5.05234)
        .CubicBezierTo(glm::vec2(16, 2.8143), glm::vec2(14.1857, 1) , glm::vec2(11.9477, 1))
        .CubicBezierTo(glm::vec2(10.7166, 1), glm::vec2(9.55233, 1.55959), glm::vec2(8.78331, 2.52086))
        .LineTo(glm::vec2(8, 3.5))
        .LineTo(glm::vec2(7.21669, 2.52086))
        .CubicBezierTo(glm::vec2(6.44767, 1.55959) , glm::vec2(5.28338, 1), glm::vec2(4.05234, 1))
        .CubicBezierTo(glm::vec2(1.8143, 1), glm::vec2(0, 2.8143), glm::vec2(0, 5.05234))
        .VerticalTo(5.24264)
        .CubicBezierTo(glm::vec2(0, 6.36786), glm::vec2(0.44699, 7.44699), glm::vec2(1.24264, 8.24264))
        .Close()
        .Build();

    m_vector_shape = m_renderer->CreateVectorShape(m_path);

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

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
    bool pathChanged = false;

    m_camera.set_viewport(m_imgui_state.scene_resolution);

    m_batch_group->Reset();
    m_batch_group->Draw(sge::VectorShapeDrawData {
        .shape = m_vector_shape
    });

    m_renderer->Begin();
    {
        sge::TemporaryFramebufferOptions tempFramebufferOpts;
        tempFramebufferOpts.colorFormat = LLGL::Format::RG11B10Float;
        tempFramebufferOpts.samples = window->GetSamples();
        tempFramebufferOpts.bindFlags = LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled | LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst;
        auto framebuffer = GetRenderContext()->GetTemporaryFramebuffer(window->GetSize(), tempFramebufferOpts);

        m_batch_group->Submit();
        m_renderer->FlushBatches();

        m_renderer->BeginPass(*framebuffer.GetRenderTarget(), m_camera);
        {
            m_renderer->Clear(LLGL::ClearValue(0.6f, 0.3f, 0.3f, 1.0f));
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

                            sge::TextureWithSampler framebufferTexture;
                            framebufferTexture.texture = framebuffer.GetTexture();
                            framebufferTexture.sampler = GetRenderContext()->GetLinearSampler()->internal().Get();
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

                        ImGui::BeginDisabled(inputBuffer.empty());
                        if (ImGui::Button("Generate path")) {
                            m_path_builder.Reset();
                            ParseSvg(inputBuffer, m_path_builder);
                            pathChanged = true;
                        }
                        ImGui::EndDisabled();

                        ImGui::PopItemWidth();

                        ImGui::SeparatorText("Current Shape Info");
                        ImGui::Text("Band Height: %f", m_vector_shape->GetBandHeight());
                        ImGui::Text("Size: (%f, %f)", m_vector_shape->GetSize().x, m_vector_shape->GetSize().y);
                        ImGui::Text("Partition Count: %u", m_vector_shape->GetPartitionCount());

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

    if (pathChanged) {
        std::optional<glm::vec2> customSize;
        if (shapeSizeX > 0.0 && shapeSizeY > 0.0)
            customSize = glm::vec2(shapeSizeX, shapeSizeY);
        m_vector_shape = m_renderer->CreateVectorShape(m_path_builder.Build(), customSize);
    }

    #if SGE_DEBUG_LAYER_ENABLED
    if (Input::Pressed(Key::C)) {
        LLGL::FrameProfile profile;
        GetRenderContext()->GetFrameProfile(&profile);
        SGE_LOG_DEBUG("Draw commands count: {}", profile.commandBufferRecord.drawCommands);
    }
    #endif
}
