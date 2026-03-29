#ifndef SGE_ENGINE_UTILS_HPP
#define SGE_ENGINE_UTILS_HPP

#include <GLFW/glfw3.h>

inline const char* glfwGetErrorString() {
    const char* description = nullptr;
    glfwGetError(&description);
    return description;
}

#endif