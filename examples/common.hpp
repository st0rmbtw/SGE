#pragma once

#ifndef EXAMPLES_COMMON_
#define EXAMPLES_COMMON_

#include <cstdint>
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
            getchar();
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

#endif