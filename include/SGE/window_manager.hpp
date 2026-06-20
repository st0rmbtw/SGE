#ifndef SGE_WINDOW_MANAGER_HPP_
#define SGE_WINDOW_MANAGER_HPP_

#include <GLFW/glfw3.h>
#include <SGE/renderer/glfw_window.hpp>
#include <expected>
#include <memory>
#include <unordered_map>

namespace sge {

using WindowMap = std::unordered_map<GLFWwindow*, std::shared_ptr<sge::GlfwWindow>>;

class WindowIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = GlfwWindow;
    using difference_type = typename WindowMap::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    explicit WindowIterator(WindowMap::iterator it) : current(std::move(it)) {   
    }

    reference operator*() const { return *current->second.get(); }
    pointer operator->() const { return current->second.get(); }

    WindowIterator& operator++() {
        ++current;
        return *this;
    }

    WindowIterator operator++(int) {
        WindowIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const WindowIterator& other) const { return current == other.current; }
    bool operator!=(const WindowIterator& other) const { return current != other.current; }

private:
    WindowMap::iterator current;
};

class WindowContainer {
public:
    WindowContainer(WindowMap::iterator begin, WindowMap::iterator end) :
        m_begin(std::move(begin)), m_end(std::move(end)) {}

    WindowIterator begin() { return WindowIterator(m_begin); }
    WindowIterator end() { return WindowIterator(m_end); }
private:
    WindowMap::iterator m_begin;
    WindowMap::iterator m_end;
};

namespace WindowManager {
    std::expected<std::shared_ptr<sge::GlfwWindow>, const char*> CreateWindow(const WindowSettings& window_settings);
    void DestroyWindow(const std::shared_ptr<sge::GlfwWindow>& window);
    WindowContainer IterWindows();
    sge::GlfwWindow* GetFocusedWindow();

    std::shared_ptr<sge::GlfwWindow> FindByHandle(GLFWwindow* handle);

    WindowMap& GetWindowMap();
}

} // namespace sge

#endif // SGE_WINDOW_MANAGER_HPP_