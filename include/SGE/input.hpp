#ifndef _SGE_INPUT_HPP_
#define _SGE_INPUT_HPP_

#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stdint.h>
#include <vector>
#include <unordered_set>

namespace sge {

namespace Modifier {
    enum : uint8_t {
        Shift = GLFW_MOD_SHIFT,
        Control = GLFW_MOD_CONTROL,
        Alt = GLFW_MOD_ALT,
        Super = GLFW_MOD_SUPER,
    };
};

enum class MouseButton : uint8_t {
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
};

enum class Key : uint16_t {
    Q = GLFW_KEY_Q,
    W = GLFW_KEY_W,
    E = GLFW_KEY_E,
    R = GLFW_KEY_R,
    T = GLFW_KEY_T,
    Y = GLFW_KEY_Y,
    U = GLFW_KEY_U,
    I = GLFW_KEY_I,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    A = GLFW_KEY_A,
    S = GLFW_KEY_S,
    D = GLFW_KEY_D,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    Z = GLFW_KEY_Z,
    X = GLFW_KEY_X,
    C = GLFW_KEY_C,
    V = GLFW_KEY_V,
    B = GLFW_KEY_B,
    N = GLFW_KEY_N,
    M = GLFW_KEY_M,

    Comma = GLFW_KEY_COMMA,
    Period = GLFW_KEY_PERIOD,
    Apostrophe = GLFW_KEY_APOSTROPHE,

    Digit1 = GLFW_KEY_1,
    Digit2 = GLFW_KEY_2,
    Digit3 = GLFW_KEY_3,
    Digit4 = GLFW_KEY_4,
    Digit5 = GLFW_KEY_5,
    Digit6 = GLFW_KEY_6,
    Digit7 = GLFW_KEY_7,
    Digit8 = GLFW_KEY_8,
    Digit9 = GLFW_KEY_9,
    Digit0 = GLFW_KEY_0,
    Minus = GLFW_KEY_MINUS,
    Equals = GLFW_KEY_EQUAL,
    Backspace = GLFW_KEY_BACKSPACE,

    LeftShift = GLFW_KEY_LEFT_SHIFT,
    RightShift = GLFW_KEY_RIGHT_SHIFT,

    LeftAlt = GLFW_KEY_LEFT_ALT,
    RightAlt = GLFW_KEY_RIGHT_ALT,

    LeftCtrl = GLFW_KEY_LEFT_CONTROL,
    RightCtrl = GLFW_KEY_RIGHT_CONTROL,

    LeftSuper = GLFW_KEY_LEFT_SUPER,
    RightSuper = GLFW_KEY_RIGHT_SUPER,

    Space = GLFW_KEY_SPACE,

    Escape = GLFW_KEY_ESCAPE,
    Tab = GLFW_KEY_TAB,
    CapsLock = GLFW_KEY_CAPS_LOCK,

    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,

    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    Pause = GLFW_KEY_PAUSE,

    ArrowUp = GLFW_KEY_UP,
    ArrowDown = GLFW_KEY_DOWN,
    ArrowLeft = GLFW_KEY_LEFT,
    ArrowRight = GLFW_KEY_RIGHT,

    End = GLFW_KEY_END,
    Home = GLFW_KEY_HOME,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,
    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN
};

struct KeyWithModifiers {
    sge::Key key;
    uint8_t modifiers;

    constexpr KeyWithModifiers(sge::Key k, uint8_t mods = 0) : key(k), modifiers(mods) {}
};

constexpr inline bool operator==(const sge::KeyWithModifiers a, const sge::KeyWithModifiers b) {
    return a.key == b.key;
}

enum class InputEventType : uint8_t {
    Key,
    MouseButton,
    CursorMove,
    Scroll,
    CodePoint
};

struct KeyInputEvent {
    sge::Key Key;
    int Mods;
    bool Pressed;
};

struct MouseButtonInputEvent {
    sge::MouseButton Button;
    bool Pressed;
};

struct ScrollInputEvent {
    float ScrollX;
    float ScrollY;
};

struct CodePointEvent {
    uint32_t CodePoint;
};

struct CursorMoveEvent {
    glm::vec2 Pos;
};

struct InputEvent {
    union {
        ScrollInputEvent ScrollEvent;
        CursorMoveEvent CursorMoveEvent;
        CodePointEvent CodePointEvent;
        KeyInputEvent KeyEvent;
        MouseButtonInputEvent MouseButtonEvent;
    };

    InputEventType Type;
};

namespace Input {
    void ProcessEvent(const InputEvent& event);

    bool Pressed(Key key);
    bool Pressed(Key key, uint8_t modifiers);
    bool JustPressed(Key key);
    bool JustPressed(Key key, uint8_t modifiers);

    bool Pressed(MouseButton button);
    bool JustPressed(MouseButton button);
    bool JustReleased(MouseButton button);

    const std::vector<float>& ScrollEvents() noexcept;
    const glm::vec2& CursorPosition() noexcept;
    const std::vector<uint32_t>& CodePoints() noexcept;
    glm::vec2 MouseDelta() noexcept;

    const std::unordered_set<KeyWithModifiers>& GetJustPressedKeys() noexcept;
    const std::unordered_set<KeyWithModifiers>& GetJustReleasedKeys() noexcept;

    void Clear();
}

}

template <>
struct std::hash<sge::KeyWithModifiers> {
    std::size_t operator()(const sge::KeyWithModifiers& key) const noexcept {
        return std::hash<sge::Key>{}(key.key);
    }
};

#endif