#include "game.hpp"

#include <GLFW/glfw3.h>

#include "constants.hpp"
#include "renderer/camera.h"
#include "renderer/renderer.hpp"
#include "time/time.hpp"
#include "assets.hpp"
#include "log.hpp"
#include "types/shape.hpp"
#include "utils.hpp"
#include "input.hpp"
#include "ui.hpp"
#include "defines.hpp"

static struct GameState {
    Camera camera;
    GLFWwindow *window = nullptr;
    bool minimized = false;
} g;

void pre_update();
void fixed_update();
void update();
void post_update();
void render();
void post_render();

static void handle_keyboard_events(GLFWwindow* window, int key, int scancode, int action, int mods);
static void handle_mouse_button_events(GLFWwindow* window, int button, int action, int mods);
static void handle_mouse_scroll_events(GLFWwindow* window, double xoffset, double yoffset);
static void handle_cursor_pos_events(GLFWwindow* window, double xpos, double ypos);
static void handle_window_resize_events(GLFWwindow* window, int width, int height);
static void handle_window_iconify_callback(GLFWwindow* window, int iconified);

GLFWwindow* create_window(LLGL::Extent2D size, bool fullscreen) {
    glfwWindowHint(GLFW_FOCUSED, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWmonitor* primary_monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(size.width, size.height, "AAA", primary_monitor, nullptr);
    if (window == nullptr) {
        LOG_ERROR("Couldn't create a window: %s", glfwGetErrorString());
        return nullptr;
    }

    glfwSetWindowSizeLimits(window, 1000, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetKeyCallback(window, handle_keyboard_events);
    glfwSetMouseButtonCallback(window, handle_mouse_button_events);
    glfwSetScrollCallback(window, handle_mouse_scroll_events);
    glfwSetCursorPosCallback(window, handle_cursor_pos_events);
    glfwSetWindowSizeCallback(window, handle_window_resize_events);
    glfwSetWindowIconifyCallback(window, handle_window_iconify_callback);

    return window;
}

bool Game::Init(RenderBackend backend, GameConfig config) {
    if (!glfwInit()) {
        LOG_ERROR("Couldn't initialize GLFW: %s", glfwGetErrorString());
        return false;
    }

    LLGL::Log::RegisterCallbackStd();

    if (!Renderer::InitEngine(backend)) return false;
    if (!Assets::Load()) return false;
    if (!Assets::LoadFonts()) return false;
    if (!Assets::InitSamplers()) return false;

    if (!Assets::LoadShaders()) return false;

    auto window_size = LLGL::Extent2D(1280, 720);
    if (config.fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window_size = LLGL::Extent2D(mode->width, mode->height);
    }

    const LLGL::Display* display = LLGL::Display::GetPrimary();
    const std::uint32_t resScale = (display != nullptr ? static_cast<std::uint32_t>(display->GetScale()) : 1u);
    const auto resolution = LLGL::Extent2D(window_size.width * resScale, window_size.height * resScale);

    GLFWwindow *window = create_window(window_size, config.fullscreen);
    if (window == nullptr) return false;

    g.window = window;
    
    if (!Renderer::Init(window, resolution, config.vsync, config.fullscreen)) return false;

    UI::Init();

    g.camera.set_viewport({window_size.width, window_size.height});
    g.camera.set_zoom(1.0f);

    glfwShowWindow(window);

    return true;
}

void Game::Run() {
    using Constants::FIXED_UPDATE_INTERVAL;
    double prev_tick = glfwGetTime();

    double fixed_timer = 0;
    
    while (Renderer::Surface()->ProcessEvents()) {
        MACOS_AUTORELEASEPOOL_OPEN
            const double current_tick = glfwGetTime();
            const double delta_time = (current_tick - prev_tick);
            prev_tick = current_tick;

            const delta_time_t dt(delta_time);
            Time::advance_by(dt);

            pre_update();

            fixed_timer += delta_time;
            while (fixed_timer >= FIXED_UPDATE_INTERVAL) {
                Time::fixed_advance_by(delta_time_t(FIXED_UPDATE_INTERVAL));
                fixed_update();
                fixed_timer -= FIXED_UPDATE_INTERVAL;
            }

            update();
            post_update();
            
            if (!g.minimized) {
                render();
                post_render();
            }

            Input::Clear();
        MACOS_AUTORELEASEPOOL_CLOSE
    }
}

void Game::Destroy() {
    if (Renderer::CommandQueue()) {
        Renderer::CommandQueue()->WaitIdle();
    }
    
    if (Renderer::Context()) {
        Renderer::Terminate();
    }

    glfwTerminate();
}

void pre_update() {

}

void fixed_update() {

}

void update() {
    for (const float scroll : Input::ScrollEvents()) {
        const float zoom_factor = glm::pow(0.75f, scroll);

        const glm::vec2 mouse_pos = g.camera.screen_to_world(Input::MouseScreenPosition());
        const glm::vec2 length = mouse_pos - g.camera.position();
        const glm::vec2 scaledLength = length * zoom_factor;
        const glm::vec2 deltaLength = length - scaledLength;

        g.camera.set_position(g.camera.position() + deltaLength);
        g.camera.set_zoom(g.camera.zoom() * zoom_factor);
    }

    if (Input::Pressed(MouseButton::Left)) {
        g.camera.set_position(g.camera.position() - Input::MouseDelta() * g.camera.zoom());
    }

    if (g.camera.needs_update()) {
        g.camera.update();
    }
}

void post_update() {

}

void render() {
    Renderer::Begin(g.camera);

    Sprite sprite;
    sprite.set_custom_size(glm::vec2(50.0f));
    sprite.set_color(glm::vec3(1.0f, 0.0f, 0.0f));
    Renderer::DrawSprite(sprite);

    Renderer::DrawShape(Shape::Circle, glm::vec2(g.camera.viewport()) / 2.0f, glm::vec2(100.0f),glm::vec4(0.5f, 0.93f, 0.5f, 1.0f), glm::vec4(1.0f), 0.1f);

    Renderer::Render(g.camera);
}

void post_render() {
    g.camera.set_mutated(false);

#if DEBUG
    if (Input::Pressed(Key::C)) {
        Renderer::PrintDebugInfo();
    }
#endif
}

static void handle_keyboard_events(GLFWwindow*, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
        Input::Press(static_cast<Key>(key));
    } else if (action == GLFW_RELEASE) {
        Input::Release(static_cast<Key>(key));
    }
}

static void handle_mouse_button_events(GLFWwindow*, int button, int action, int) {
    if (action == GLFW_PRESS) {
        Input::press(static_cast<MouseButton>(button));
    } else if (action == GLFW_RELEASE) {
        Input::release(static_cast<MouseButton>(button));
    }
}

static void handle_mouse_scroll_events(GLFWwindow*, double, double yoffset) {
    Input::PushMouseScrollEvent(yoffset);
}

static void handle_cursor_pos_events(GLFWwindow*, double xpos, double ypos) {
    const glm::uvec2 window_size = g.camera.viewport();

    xpos = std::min(std::max(xpos, 0.0), static_cast<double>(window_size.x));
    ypos = std::min(std::max(ypos, 0.0), static_cast<double>(window_size.y));

    Input::SetMouseScreenPosition(glm::vec2(xpos, ypos));
}

static void handle_window_resize_events(GLFWwindow*, int width, int height) {
    if (width <= 0 || height <= 0) {
        g.minimized = true;
        return;
    } else {
        g.minimized = false;
    }

    const LLGL::Display* display = LLGL::Display::GetPrimary();
    const std::uint32_t resScale = (display != nullptr ? static_cast<std::uint32_t>(display->GetScale()) : 1u);

    const auto new_size = LLGL::Extent2D(width * resScale, height * resScale);

    Renderer::CommandQueue()->WaitIdle();
    Renderer::SwapChain()->ResizeBuffers(new_size);

    g.camera.set_viewport({width, height});

    render();
}

static void handle_window_iconify_callback(GLFWwindow*, int iconified) {
    g.minimized = iconified;
}