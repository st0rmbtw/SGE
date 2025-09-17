#include <unordered_set>
#include <SGE/input.hpp>

struct KeyWithModifiers {
    sge::Key key;
    uint8_t modifiers;

    constexpr KeyWithModifiers(sge::Key k, uint8_t mods = 0) : key(k), modifiers(mods) {}
};

constexpr inline bool operator==(const KeyWithModifiers a, const KeyWithModifiers b) {
    return a.key == b.key;
}

template <>
struct std::hash<KeyWithModifiers> {
    std::size_t operator()(const KeyWithModifiers& key) const noexcept {
        return std::hash<sge::Key>{}(key.key);
    }
};

namespace sge {

static struct InputState {
    std::unordered_set<KeyWithModifiers> keyboard_pressed;
    std::unordered_set<KeyWithModifiers> keyboard_just_pressed;
    std::unordered_set<KeyWithModifiers> keyboard_just_released;

    std::unordered_set<uint8_t> mouse_pressed;
    std::unordered_set<uint8_t> mouse_just_pressed;
    std::unordered_set<uint8_t> mouse_just_released;
    std::vector<float> mouse_scroll_events;
    glm::vec2 mouse_screen_position;
    glm::vec2 mouse_delta;
} input_state;

void Input::Press(Key key, uint8_t modifiers) {
    const KeyWithModifiers value = KeyWithModifiers(key, modifiers);
    if (input_state.keyboard_pressed.insert(value).second) {
        input_state.keyboard_just_pressed.insert(value);
    }
}

void Input::Release(Key key, uint8_t modifiers) {
    const KeyWithModifiers value = KeyWithModifiers(key, modifiers);
    if (input_state.keyboard_pressed.erase(value) > 0) {
        input_state.keyboard_just_released.insert(value);
    }
}

bool Input::Pressed(Key key) {
    return input_state.keyboard_pressed.find(key) != input_state.keyboard_pressed.end();
}

bool Input::JustPressed(Key key) {
    return input_state.keyboard_just_pressed.find(key) != input_state.keyboard_just_pressed.end();
}

bool Input::Pressed(Key key, uint8_t modifiers) {
    const auto entry = input_state.keyboard_pressed.find(key);
    if (entry == input_state.keyboard_pressed.end()) return false;
    return entry->modifiers == modifiers;
}

bool Input::JustPressed(Key key, uint8_t modifiers) {
    const auto entry = input_state.keyboard_just_pressed.find(key);
    if (entry == input_state.keyboard_just_pressed.end()) return false;
    return entry->modifiers == modifiers;
}

void Input::Clear() {
    input_state.keyboard_just_pressed.clear();
    input_state.keyboard_just_released.clear();

    input_state.mouse_just_pressed.clear();
    input_state.mouse_just_released.clear();
    input_state.mouse_scroll_events.clear();
    input_state.mouse_delta = glm::vec2(0.0f);
}


void Input::Press(MouseButton button) {
    if (input_state.mouse_pressed.insert(static_cast<uint8_t>(button)).second) {
        input_state.mouse_just_pressed.insert(static_cast<uint8_t>(button));
    }
}

void Input::Release(MouseButton button) {
    if (input_state.mouse_pressed.erase(static_cast<uint8_t>(button)) > 0) {
        input_state.mouse_just_released.insert(static_cast<uint8_t>(button));
    }
}

bool Input::Pressed(MouseButton button) {
    return input_state.mouse_pressed.find(static_cast<uint8_t>(button)) != input_state.mouse_pressed.end();
}

bool Input::JustPressed(MouseButton button) {
    return input_state.mouse_just_pressed.find(static_cast<uint8_t>(button)) != input_state.mouse_just_pressed.end();
}

bool Input::JustReleased(MouseButton button) {
    return input_state.mouse_just_released.find(static_cast<uint8_t>(button)) != input_state.mouse_just_released.end();
}

void Input::PushMouseScrollEvent(float y) noexcept {
    input_state.mouse_scroll_events.push_back(y);
}

void Input::SetMouseScreenPosition(const glm::vec2& position) noexcept {
    input_state.mouse_delta = position - input_state.mouse_screen_position;
    input_state.mouse_screen_position = position;
}

glm::vec2 Input::MouseDelta() noexcept {
    return input_state.mouse_delta;
}

const std::vector<float>& Input::ScrollEvents() noexcept {
    return input_state.mouse_scroll_events;
}

const glm::vec2& Input::MouseScreenPosition() noexcept {
    return input_state.mouse_screen_position;
}

}