#ifndef _SGE_TYPES_CURSOR_MODE_HPP_
#define _SGE_TYPES_CURSOR_MODE_HPP_

#include <GLFW/glfw3.h>

namespace sge {

enum class CursorMode : int {
    Normal = GLFW_CURSOR_NORMAL,
    Hidden = GLFW_CURSOR_HIDDEN,
    Disabled = GLFW_CURSOR_DISABLED,
    Captured = GLFW_CURSOR_CAPTURED,
};

}

#endif
