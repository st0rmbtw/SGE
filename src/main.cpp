#include <cstring>
#include <cstdio>
#include "game.hpp"
#include "engine/defines.hpp"

using namespace sge;

inline void print_render_backends() {
    #if defined(PLATFORM_WINDOWS)
        printf("Available render backends: d3d11, d3d12, opengl, vulkan.\n");
    #elif defined(PLATFORM_MACOS)
        printf("Available render backends: metal, opengl, vulkan.\n");
    #elif defined(PLATFORM_LINUX)
        printf("Available render backends: opengl, vulkan.\n");
    #endif
}

#define str_eq(a, b) strcmp(a, b) == 0

int main(int argc, char** argv) {
#if defined(PLATFORM_WINDOWS)
    types::RenderBackend backend = types::RenderBackend::D3D11;
#elif defined(PLATFORM_MACOS)
    types::RenderBackend backend = types::RenderBackend::Metal;
#else
    types::RenderBackend backend = types::RenderBackend::OpenGL;
#endif
    types::AppConfig config;

    for (int i = 1; i < argc; i++) {
        if (str_eq(argv[i], "--pause")) {
            printf("Initialization is paused. Press any key to continue...\n");
            getchar();
        } else if (str_eq(argv[i], "--backend")) {
            if (i >= argc-1) {
                printf("Specify a render backend. ");
                print_render_backends();
                return 1;
            }

            const char* arg = argv[i + 1];

            if (str_eq(arg, "vulkan")) {
                backend = types::RenderBackend::Vulkan;
            } else

            #ifdef PLATFORM_WINDOWS
            if (str_eq(arg, "d3d12")) {
                backend = types::RenderBackend::D3D12;
            } else
            
            if (str_eq(arg, "d3d11")) {
                backend = types::RenderBackend::D3D11;
            } else
            #endif

            #ifdef PLATFORM_MACOS
            if (str_eq(arg, "metal")) {
                backend = types::RenderBackend::Metal;
            } else
            #endif

            if (str_eq(arg, "opengl")) {
                backend = types::RenderBackend::OpenGL;
            } else {
                printf("Unknown render backend: \"%s\". ", arg);
                print_render_backends();
                return 1;
            }
        } else if (str_eq(argv[i], "--vsync")) {
            config.vsync = true;
        } else if (str_eq(argv[i], "--fullscreen")) {
            config.fullscreen = true;
        }
    }

    if (Game::Init(backend, config)) {
        Game::Run();
    }
    Game::Destroy();

    return 0;
}