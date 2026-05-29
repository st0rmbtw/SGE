#include <SGE/window_manager.hpp>
#include "utils.hpp"

static struct {
    sge::WindowMap window_map;
} state;

std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> sge::WindowManager::CreateWindow(const WindowSettings& window_settings) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_VISIBLE, window_settings.hidden ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, window_settings.focused ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, window_settings.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, window_settings.decorated ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* primary_monitor = window_settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

    GLFWwindow *window = glfwCreateWindow(window_settings.width, window_settings.height, window_settings.title, primary_monitor, nullptr);
    if (window == nullptr) {
        return std::unexpected(glfwGetErrorString());
    }

    glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(window_settings.cursor_mode));

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::ivec2 position;
    glfwGetWindowPos(window, &position.x, &position.y);

    const uint8_t vsync_interval = window_settings.vsync;

    std::shared_ptr<GlfwWindow> instance = std::make_shared<GlfwWindow>(window, LLGL::Extent2D(width, height), position, window_settings.cursor_mode, window_settings.samples, vsync_interval, window_settings.fullscreen);
    state.window_map.try_emplace(window, instance);
    return instance;
}

void sge::WindowManager::DestroyWindow(const std::shared_ptr<sge::GlfwWindow>& window) {
    state.window_map.erase(window->GetGlfwHandle());
}

sge::WindowContainer sge::WindowManager::IterWindows() {
    return WindowContainer(state.window_map.begin(), state.window_map.end());
}

sge::GlfwWindow* sge::WindowManager::GetFocusedWindow() {
    for (const auto& [ptr, window] : state.window_map) {
        if (window->IsFocused()) return window.get();
    }
    return nullptr;
}

std::shared_ptr<sge::GlfwWindow> sge::WindowManager::FindByHandle(GLFWwindow* handle) {
    auto it = state.window_map.find(handle);
    if (it == state.window_map.end()) {
        return nullptr;
    }
    return it->second;
}

sge::WindowMap& sge::WindowManager::GetWindowMap() {
    return state.window_map;
}