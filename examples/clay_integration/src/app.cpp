#include "app.hpp"

#include <SGE/engine.hpp>
#include <SGE/renderer/renderer.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/input.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/anchor.hpp>
#include <SGE/types/color.hpp>
#include <SGE/types/window_settings.hpp>
#include <SGE/types/blend_mode.hpp>

#include <glm/trigonometric.hpp>

#define CLAY_IMPLEMENTATION
#include "clay.h"

static constexpr double FIXED_UPDATE_INTERVAL = 1.0 / 60.0;

using namespace sge;

static void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

App::App(const ExampleConfig& config) : m_camera(config.backend, CameraOrigin::TopLeft) {
    InitRenderContext(config.backend);

    WindowSettings window_settings;
    window_settings.width = 1280;
    window_settings.height = 720;
    window_settings.fullscreen = config.fullscreen;
    window_settings.vsync = config.vsync;
    window_settings.samples = config.samples;
    window_settings.hidden = true;

    auto result = CreateWindow(window_settings);
    if (!result.has_value()) {
        SGE_LOG_ERROR("Couldn't create a window: {}", result.error());
        std::abort();
    }

    GlfwWindow* window = result.value();
    m_primary_window_id = window->GetID();

    LLGL::Extent2D resolution = window->GetContentSize();
    m_camera.set_viewport({resolution.width, resolution.height});
    m_camera.set_zoom(1.0f);

    m_renderer = std::make_unique<Renderer>(GetRenderContext());

    m_batch = m_renderer->CreateBatch();
    m_batch->SetIsUi(true);
    m_batch->BeginBlendMode(sge::BlendMode::PremultipliedAlpha);

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char *)malloc(totalMemorySize));
    Clay_Initialize(
        clayMemory,
        Clay_Dimensions{ .width=static_cast<float>(resolution.width), .height=static_cast<float>(resolution.height) },
        Clay_ErrorHandler{ .errorHandlerFunction = HandleClayErrors, .userData = NULL }
    );

    Time::SetFixedTimestepSeconds(FIXED_UPDATE_INTERVAL);

    window->ShowWindow();
}

App::~App() {
    m_renderer->DestroyBatch(*m_batch);
}

void App::OnUpdate() {
    Clay_SetLayoutDimensions(Clay_Dimensions {
        .width = static_cast<float>(m_camera.viewport().x),
        .height = static_cast<float>(m_camera.viewport().y)
    });

    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);
        const float new_zoom = m_camera.zoom() * zoom_factor;

        m_camera.set_zoom(glm::clamp(new_zoom, 0.0f, 1.0f));

        const glm::vec2 mouse_pos = m_camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - m_camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        const Rect& area = m_camera.get_projection_area();
        const glm::vec2 window_size = m_camera.viewport();

        const glm::vec2 new_position = m_camera.position() + deltaLength;
        m_camera.set_position(glm::clamp(new_position, glm::vec2(0.0f), glm::vec2(window_size - area.size())));
    }

    if (Input::Pressed(MouseButton::Left)) {
        const Rect& area = m_camera.get_projection_area();
        const glm::vec2 half_screen_size = glm::vec2(m_camera.viewport()) / 2.0f;

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

void App::OnRender(const std::shared_ptr<sge::GlfwWindow> &window) {
    m_renderer->Begin();

    Clay_BeginLayout();

    CLAY({
        .layout = {
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 10, 10, 10, 10 },
            .childGap = 0,
        },
        .backgroundColor = { 25, 25, 25, 255 }
    }) {
        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            },
            .backgroundColor = { 255, 0, 0, 255 },
            .cornerRadius = { 18, 0, 18, 0 },
        });

        CLAY({
            .layout = {
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            },
            .backgroundColor = { 0, 255, 0, 255 },
            .cornerRadius = { 0, 18, 0, 18 },
        });
    }

    Clay_RenderCommandArray drawCommands = Clay_EndLayout();
    for (int i = 0; i < drawCommands.length; ++i) {
        Clay_RenderCommand* drawCommand = &drawCommands.internalArray[i];
        Clay_BoundingBox boundingBox = drawCommand->boundingBox;

        switch (drawCommand->commandType) {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleRenderData *config = &drawCommand->renderData.rectangle;
            const glm::vec2 position = glm::vec2(boundingBox.x, boundingBox.y);
            const glm::vec2 size = glm::vec2(boundingBox.width, boundingBox.height);
            const LinearRgba color = LinearRgba(config->backgroundColor.r / 255.0f, config->backgroundColor.g / 255.0f, config->backgroundColor.b / 255.0f, config->backgroundColor.a / 255.0f);
            const glm::vec4 cornerRadius = glm::vec4(config->cornerRadius.topLeft, config->cornerRadius.topRight, config->cornerRadius.bottomLeft, config->cornerRadius.bottomRight);

            m_batch->DrawRect(position, {
                .size = size,
                .color = color,
                .border_thickness = 0.0f,
                .border_color = color,
                .border_radius = cornerRadius,
                .anchor = Anchor::TopLeft
            });
        } break;
        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderRenderData *config = &drawCommand->renderData.border;

            const LinearRgba color = LinearRgba(config->color.r / 255.0f, config->color.g / 255.0f, config->color.b / 255.0f, config->color.a / 255.0f);

            const glm::vec4 cornerRadius = glm::vec4(0.0f);

            // Left border
            if (config->width.left > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x, boundingBox.y + config->cornerRadius.topLeft);
                const glm::vec2 size = glm::vec2(config->width.left, boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft);

                m_batch->DrawRect(position, {
                    .size = size,
                    .color = color,
                    .border_thickness = 0.0f,
                    .border_color = LinearRgba::black(),
                    .border_radius = cornerRadius,
                    .anchor = Anchor::TopLeft
                });
            }
            // Right border
            if (config->width.right > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->width.right, boundingBox.y + config->cornerRadius.topRight);
                const glm::vec2 size = glm::vec2(config->width.right, boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight);

                m_batch->DrawRect(position, {
                    .size = size,
                    .color = color,
                    .border_thickness = 0.0f,
                    .border_color = LinearRgba::black(),
                    .border_radius = cornerRadius,
                    .anchor = Anchor::TopLeft
                });
            }
            // Top border
            if (config->width.top > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.topLeft, boundingBox.y);
                const glm::vec2 size = glm::vec2(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight, config->width.top);

                m_batch->DrawRect(position, {
                    .size = size,
                    .color = color,
                    .border_thickness = 0.0f,
                    .border_color = LinearRgba::black(),
                    .border_radius = cornerRadius,
                    .anchor = Anchor::TopLeft
                });
            }

            // Bottom border
            if (config->width.bottom > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->width.bottom);
                const glm::vec2 size = glm::vec2(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight, config->width.bottom);

                m_batch->DrawRect(position, {
                    .size = size,
                    .color = color,
                    .border_thickness = 0.0f,
                    .border_color = LinearRgba::black(),
                    .border_radius = cornerRadius,
                    .anchor = Anchor::TopLeft
                });
            }

            if (config->cornerRadius.topLeft > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.topLeft, boundingBox.y + config->cornerRadius.topLeft);
                m_batch->DrawArc(position, {
                    .outer_radius = config->cornerRadius.topLeft,
                    .inner_radius = config->cornerRadius.topLeft - config->width.top,
                    .start_angle = glm::radians(90.0f),
                    .end_angle = glm::radians(180.0f),
                    .color = color,
                    .anchor = Anchor::TopLeft
                });
            }

            if (config->cornerRadius.topRight > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->cornerRadius.topRight, boundingBox.y + config->cornerRadius.topRight);
                m_batch->DrawArc(position, {
                    .outer_radius = config->cornerRadius.topRight,
                    .inner_radius = config->cornerRadius.topRight - config->width.top,
                    .start_angle = glm::radians(0.0f),
                    .end_angle = glm::radians(90.0f),
                    .color = color,
                    .anchor = Anchor::TopLeft
                });
            }

            if (config->cornerRadius.bottomLeft > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft);
                m_batch->DrawArc(position, {
                    .outer_radius = config->cornerRadius.bottomLeft,
                    .inner_radius = config->cornerRadius.bottomLeft - config->width.bottom,
                    .start_angle = glm::radians(180.0f),
                    .end_angle = glm::radians(270.0f),
                    .color = color,
                    .anchor = Anchor::TopLeft
                });
            }

            if (config->cornerRadius.bottomRight > 0) {
                const glm::vec2 position = glm::vec2(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight, boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight);
                m_batch->DrawArc(position, {
                    .outer_radius = config->cornerRadius.bottomRight,
                    .inner_radius = config->cornerRadius.bottomRight - config->width.bottom,
                    .start_angle = glm::radians(270.0f),
                    .end_angle = glm::radians(360.0f),
                    .color = color,
                    .anchor = Anchor::TopLeft
                });
            }
        }
        break;
        case CLAY_RENDER_COMMAND_TYPE_NONE:
        case CLAY_RENDER_COMMAND_TYPE_TEXT:
        case CLAY_RENDER_COMMAND_TYPE_IMAGE:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
        case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
          break;
        }
    }

    const glm::vec2 center = m_camera.screen_center();
    m_batch->DrawLine(center, center + glm::vec2(100.0), 2.0, sge::LinearRgba::blue());

    m_batch->DrawRect(center, {
        .size = glm::vec2(250.0f),
        .color = sge::LinearRgba(0.2f, 0.2f, 0.9f),
        .border_thickness = 2.0f,
        .border_color = sge::LinearRgba::blue(),
        .border_radius = glm::vec4(14.0f)
    });

    m_renderer->BeginPass(window, m_camera);
        m_renderer->Clear(LLGL::ClearValue(0.0f, 0.0f, 0.0f, 0.0f));

        m_renderer->PrepareBatch(*m_batch);
        m_renderer->UploadBatchData();
        m_renderer->RenderBatch(*m_batch);

        m_batch->Reset();
    m_renderer->EndPass();

    m_renderer->End();
    GetRenderContext()->Present(window);
}

void App::OnPostRender(const std::shared_ptr<sge::GlfwWindow> &window) {
#if SGE_DEBUG
    if (Input::Pressed(Key::C)) {
        GetRenderContext()->PrintDebugInfo();
    }
#endif
}

void App::OnWindowResized(const std::shared_ptr<sge::GlfwWindow> &window, int width, int height) {
    m_camera.set_viewport(glm::uvec2(width, height));
    m_camera.update();
    OnRender(window);
}