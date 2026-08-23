#pragma once

#ifndef EXAMPLES_COMMON_
#define EXAMPLES_COMMON_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <SGE/input.hpp>
#include <SGE/renderer/camera.hpp>
#include <SGE/time/time.hpp>
#include <SGE/types/backend.hpp>

inline constexpr sge::RenderBackend GetDefaultBackend() {
#if SGE_PLATFORM_WINDOWS
    return sge::RenderBackend::D3D11;
#elif SGE_PLATFORM_MACOS
    return sge::RenderBackend::Metal;
#else
    return sge::RenderBackend::OpenGL;
#endif
}

struct ExampleConfig {
    bool vsync = false;
    bool fullscreen = false;
    uint8_t samples = 4;
    sge::RenderBackend backend = GetDefaultBackend();
};

inline void PrintRenderBackends() {
#if SGE_PLATFORM_WINDOWS
    printf("Available render backends: d3d11, d3d12, opengl, vulkan.\n");
#elif SGE_PLATFORM_MACOS
    printf("Available render backends: metal, opengl, vulkan.\n");
#elif SGE_PLATFORM_LINUX
    printf("Available render backends: opengl, vulkan.\n");
#endif
}

inline bool ParseCommandLineArguments(int argc, char** argv, ExampleConfig& config) {
    const auto strequal = [](const char* a, const char* b) {
        return strcmp(a, b) == 0;
    };

    for (int i = 1; i < argc; i++) {
        if (strequal(argv[i], "--pause")) {
            printf("Initialization is paused. Press any key to continue...\n");
            (void)getchar();
        } else if (strequal(argv[i], "--backend")) {
            if (i >= argc-1) {
                printf("Specify a render backend. ");
                PrintRenderBackends();
                return false;
            }

            const char* arg = argv[i + 1];

            if (strequal(arg, "vulkan")) {
                config.backend = sge::RenderBackend::Vulkan;
            } else

            #ifdef SGE_PLATFORM_WINDOWS
            if (strequal(arg, "d3d12")) {
                config.backend = sge::RenderBackend::D3D12;
            } else

            if (strequal(arg, "d3d11")) {
                config.backend = sge::RenderBackend::D3D11;
            } else
            #endif

            #ifdef SGE_PLATFORM_MACOS
            if (strequal(arg, "metal")) {
                config.backend = sge::RenderBackend::Metal;
            } else
            #endif

            if (strequal(arg, "opengl")) {
                config.backend = sge::RenderBackend::OpenGL;
            } else {
                printf("Unknown render backend: \"%s\". ", arg);
                PrintRenderBackends();
                return false;
            }
        } else if (strequal(argv[i], "--vsync")) {
            config.vsync = true;
        } else if (strequal(argv[i], "--fullscreen")) {
            config.fullscreen = true;
        } else if (strequal(argv[i], "--samples")) {
            if (i >= argc-1) {
                printf("Specify the number of samples.\n");
                return false;
            }

            const char* arg = argv[i + 1];
            config.samples = std::stoul(arg);
        }
    }

    return true;
}

inline void ControlCamera2D(sge::Camera& camera) {
    namespace Input = sge::Input;
    namespace Time = sge::Time;

    sge::Transform& camera_transform = camera.transform();

    for (const float scroll : Input::ScrollEvents()) {
        const float new_zoom = glm::clamp(camera.zoom() * glm::pow(0.75f, scroll), 0.f, 10.f);
        const float zoodelta = new_zoom / camera.zoom();

        camera.set_zoom(new_zoom);

        const glm::vec2 mouse_pos = camera.screen_to_world(Input::CursorPosition());
        const glm::vec2 length = mouse_pos - glm::vec2(camera_transform.translation);
        const glm::vec2 scaled_length = length * zoodelta;
        const glm::vec2 delta_length = length - scaled_length;

        const sge::Rect& area = camera.get_projection_area();
        const glm::vec2 window_size = glm::vec2(camera.viewport());

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + delta_length;
        camera.set_position(new_position);
    }

    if (Input::Pressed(sge::MouseButton::Left)) {
        const sge::Rect& area = camera.get_projection_area();

        const glm::vec2 new_position = glm::vec2(camera_transform.translation) + Input::MouseDelta() * camera.zoom() * glm::vec2(-1.f, -1.f);
        camera.set_position(new_position);
    }

    if (Input::Pressed(sge::Key::Escape)) {
        camera.set_zoom(1.f);
        camera.set_position(glm::vec2(0.f));
    }

    if (Input::Pressed(sge::Key::Minus)) {
        float zoom = camera.zoom() + 5.f * Time::DeltaSeconds();
        camera.set_zoom(zoom);
    }

    if (Input::Pressed(sge::Key::Equals)) {
        float zoom = camera.zoom() - 5.f * Time::DeltaSeconds();
        camera.set_zoom(zoom);
    }

    constexpr float BASE_MOVE_SPEED = 1200.0f;

    const float move_speed_koef = camera.zoom();

    if (Input::Pressed(sge::Key::W)) {
        glm::vec2 position = camera.transform().translation;
        position.y -= move_speed_koef * BASE_MOVE_SPEED * Time::DeltaSeconds();
        camera.set_position(position);
    }

    if (Input::Pressed(sge::Key::S)) {
        glm::vec2 position = camera.transform().translation;
        position.y += move_speed_koef * BASE_MOVE_SPEED * Time::DeltaSeconds();
        camera.set_position(position);
    }

    if (Input::Pressed(sge::Key::A)) {
        glm::vec2 position = camera.transform().translation;
        position.x -= move_speed_koef * BASE_MOVE_SPEED * Time::DeltaSeconds();
        camera.set_position(position);
    }

    if (Input::Pressed(sge::Key::D)) {
        glm::vec2 position = camera.transform().translation;
        position.x += move_speed_koef * BASE_MOVE_SPEED * Time::DeltaSeconds();
        camera.set_position(position);
    }
}

#endif
