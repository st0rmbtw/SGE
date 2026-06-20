#include "app.hpp"
#include "SGE/types/binding_layout.hpp"
#include "shaders.hpp"

#include <SGE/engine.hpp>
#include <SGE/input.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/attributes.hpp>
#include <SGE/types/blend_mode.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/cursor_mode.hpp>
#include <SGE/types/framebuffer.hpp>
#include <SGE/types/shape.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/window_manager.hpp>

#if SGE_IMGUI_ENABLED
    #include <imgui.h>
#endif

#include <glm/trigonometric.hpp>

namespace Input = sge::Input;
namespace Time = sge::Time;
using Key = sge::Key;

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

bool App::OnInit() {
    sge::ImGuiConfig imguiConfig;
    imguiConfig.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiConfig.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!InitRenderContext(m_config.backend, imguiConfig))
        return false;

    SetAutoPresent(true);

    sge::WindowSettings window_settings;
    window_settings.width = 1280;
    window_settings.height = 720;
    window_settings.fullscreen = m_config.fullscreen;
    window_settings.vsync = m_config.vsync;
    window_settings.samples = m_config.samples;
    window_settings.hidden = true;
    window_settings.cursor_mode = sge::CursorMode::Disabled;

    auto result = CreateWindow(window_settings);
    if (!result.has_value()) {
        SGE_LOG_ERROR("Couldn't create a window: {}", result.error());
        return false;
    }

    std::shared_ptr<sge::GlfwWindow> window = result.value();

    LLGL::Extent2D resolution = window->GetContentSize();

    m_renderer = std::make_unique<sge::Renderer2D>(GetRenderContext());

    m_uniforms.projection_matrix = glm::perspectiveLH_ZO(glm::radians(45.0f), 1280.0f / 720.0f, 0.001f, 100.0f);

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    InitPipeline();
    InitFramebuffer(window->GetSize());

    window->ShowWindow();

    return true;
}

void App::InitPipeline() {
    const auto& context = GetRenderContext();

    LLGL::PipelineLayoutDescriptor layoutDesc;
    layoutDesc.bindings = sge::BindingLayout({
        sge::BindingLayoutItem::ConstantBuffer(2, "UniformConstantBuffer", LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage)
    });

    LLGL::VertexFormat vertexFormat = sge::Attributes(context->Backend(), {
        sge::Attribute::Vertex(LLGL::Format::RGB32Float, "a_position", "Position")
    });

    ShaderSourceCode sourceCode = GetBasicShaderSourceCode(GetRenderContext()->Backend());

    sge::GraphicsPipelineConfig pipelineConfig;
    pipelineConfig.layout = context->CreatePipelineLayout(layoutDesc);
    pipelineConfig.vertexShader = context->CreateShader(sge::ShaderType::Vertex, "VS", sourceCode.vs_source, sourceCode.vs_size, vertexFormat.attributes);
    pipelineConfig.pixelShader = context->CreateShader(sge::ShaderType::Fragment, "PS", sourceCode.fs_source, sourceCode.fs_size);
    pipelineConfig.cullMode = LLGL::CullMode::Back;

    m_pipeline_handle = context->CreatePipelineState(pipelineConfig);

    const glm::vec3 vertices[] = {
        // 1. BACK FACE
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),

        // 2. FRONT FACE
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),

        // 3. LEFT FACE
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),

        // 4. RIGHT FACE
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),

        // 5. BOTTOM FACE
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),

        // 6. TOP FACE
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f)
    };

    m_vertex_buffer = context->CreateVertexBuffer(vertices, vertexFormat);

    pipelineConfig = {};
    pipelineConfig.layout = m_renderer->BlitPipelineLayout();
    pipelineConfig.vertexShader = m_renderer->FullscreenTriangleVertexShader();
    pipelineConfig.pixelShader = m_renderer->BlitShader();
    m_postprocess_pipeline_handle = context->CreatePipelineState(pipelineConfig);

    m_uniform_buffer = GetRenderContext()->CreateConstantBuffer(sizeof(UniformBuffer));
}

void App::InitFramebuffer(LLGL::Extent2D resolution) {
    sge::FramebufferConfig framebufferConfig;
    framebufferConfig.format = LLGL::Format::RG11B10Float;
    framebufferConfig.resolution = resolution;
    framebufferConfig.colorAttachments[0].format = LLGL::Format::RG11B10Float;
    framebufferConfig.colorAttachments[0].bindFlags |= LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst;
    m_postprocess_framebuffer = std::make_unique<sge::Framebuffer>(GetRenderContext()->CreateFramebuffer(framebufferConfig));
}

void App::OnUpdate() {
    if (Input::JustPressed(Key::I)) {
        m_render_imgui = !m_render_imgui;
    }

    sge::GlfwWindow* window = sge::WindowManager::GetFocusedWindow();
    if (window != nullptr) {
        const bool cursorDisabled = window->GetCursorMode() == sge::CursorMode::Disabled;

        if (Input::JustPressed(Key::Escape)) {
            window->SetCursorMode(cursorDisabled ? sge::CursorMode::Normal : sge::CursorMode::Disabled);
        }

        if (!cursorDisabled)
            return;
    }

    const float dt = Time::DeltaSeconds();

    bool changed = false;

    float prev_yaw = m_yaw;
    float prev_pitch = m_pitch;

    m_yaw += Input::MouseDelta().x * 0.05f;
    m_pitch += Input::MouseDelta().y * 0.05f;

    if (!sge::approx_equals(prev_yaw, m_yaw))
        changed = true;
    if (!sge::approx_equals(prev_pitch, m_pitch))
        changed = true;

    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    if (changed) {
        m_yaw = std::fmod(m_yaw, 360.0f);
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

        glm::vec3 cameraDir;
        cameraDir.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        cameraDir.y = glm::sin(glm::radians(m_pitch));
        cameraDir.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        m_camera_forward = glm::normalize(cameraDir);
    }

    float speed = 1.0f;
    if (Input::Pressed(sge::Key::LeftCtrl)) {
        speed *= 0.075f;
    }

    if (Input::Pressed(sge::Key::W)) {
        m_camera_pos += m_camera_forward * speed * dt;
        changed = true;
    }
    if (Input::Pressed(sge::Key::S)) {
        m_camera_pos -= m_camera_forward * speed * dt;
        changed = true;
    }
    if (Input::Pressed(sge::Key::D)) {
        glm::vec3 right = glm::normalize(glm::cross(up, m_camera_forward));
        m_camera_pos += right * speed * dt;
        changed = true;
    }
    if (Input::Pressed(sge::Key::A)) {
        glm::vec3 right = glm::normalize(glm::cross(up, m_camera_forward));
        m_camera_pos -= right * speed * dt;
        changed = true;
    }
    if (Input::Pressed(sge::Key::Space)) {
        m_camera_pos += up * speed * dt;
        changed = true;
    }
    if (Input::Pressed(sge::Key::LeftShift)) {
        m_camera_pos -= up * speed * dt;
        changed = true;
    }

    if (changed) {
        m_uniforms.view_matrix = glm::lookAtLH(m_camera_pos, m_camera_pos + m_camera_forward, up);
    }
}

void App::OnRender(const std::shared_ptr<sge::GlfwWindow>& window) {
    const auto& context = GetRenderContext();
    auto* commands = m_renderer->CommandBuffer();

    m_renderer->Begin();
    {
        commands->UpdateBuffer(*m_uniform_buffer, 0, &m_uniforms, sizeof(UniformBuffer));

        m_renderer->BeginPass(*m_postprocess_framebuffer->GetRenderTarget());
        {
            m_renderer->Clear(LLGL::ClearValue(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f));
            commands->SetViewport(m_postprocess_framebuffer->GetResolution());

            commands->SetVertexBuffer(*m_vertex_buffer);
            commands->SetPipelineState(context->GetOrCreatePipeline(m_pipeline_handle));
            commands->SetResource(0, *m_uniform_buffer);
            commands->Draw(36, 0);
        }
        m_renderer->EndPass();

        m_renderer->BloomPass(*m_postprocess_framebuffer, m_bloom_settings);
        m_renderer->TonemapPass(*m_postprocess_framebuffer);

        m_renderer->BeginPass(window);
        {
            commands->SetViewport(window->GetSize());
            commands->SetPipelineState(context->GetOrCreatePipeline(m_postprocess_pipeline_handle));
            commands->SetVertexBuffer(*m_renderer->FullscreenTriangleVertexBuffer());
            commands->SetResource(0, *m_postprocess_framebuffer->GetTexture(0));
            commands->SetResource(1, *context->GetNearestSampler());
            commands->Draw(3, 0);

            #if SGE_IMGUI_ENABLED
            if (m_render_imgui) {
                GetRenderContext()->BeginImGuiFrame(*window);
                {
                    ImGui::NewFrame();
                    {
                        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
                        ImGui::Begin("Bloom");
                        {
                            ImGui::DragFloat("Threshold", &m_bloom_settings.threshold, 0.05f, 0.0f, 1.0f);
                            ImGui::DragFloat("Knee", &m_bloom_settings.knee, 0.01f, 0.0001f, 1.0f);
                            ImGui::DragFloat("Intensity", &m_bloom_settings.intensity, 0.01f, 0.0f, 10.0f);
                            ImGui::DragFloat("Scatter", &m_bloom_settings.scatter, 0.1f, 0.0f, 10.0f);
                            ImGui::DragScalar("Max Iterations", ImGuiDataType_U8, &m_bloom_settings.maxIterations);
                            ImGui::ColorPicker3("Clear Color", &m_clear_color.r);
                            ImGui::ColorPicker3("Object Color", &m_uniforms.object_color.r, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker);
                        }
                        ImGui::End();
                    }
                    ImGui::Render();
                }
                GetRenderContext()->EndImGuiFrame();
            }
            #endif
        }
        m_renderer->EndPass();
    }
    m_renderer->End();
}