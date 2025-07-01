#include <unordered_set>
#include <SGE/input.hpp>

struct KeyWithModifiers {
    sge::Key key;
    uint8_t modifiers;

    constexpr KeyWithModifiers(sge::Key k, uint8_t mods = 0) : key(k), modifiers(mods) {}
};

constexpr inline bool operator==(const KeyWithModifiers a, const KeyWithModifiers b) {
    return a.key == b.key && a.modifiers == b.modifiers;
}

template <>
struct std::hash<KeyWithModifiers> {
    std::size_t operator()(const KeyWithModifiers& key) const noexcept {
        std::size_t h1 = std::hash<sge::Key>{}(key.key);
        std::size_t h2 = std::hash<uint8_t>{}(key.modifiers);
        return h1 ^ (h2 << 1);
    }
};

_SGE_BEGIN

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
    bool mouse_over_ui = false;
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
    input_state.mouse_over_ui = false;
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

void Input::PushMouseScrollEvent(float y) { input_state.mouse_scroll_events.push_back(y); }
void Input::SetMouseScreenPosition(const glm::vec2& position) {
    input_state.mouse_delta = position - input_state.mouse_screen_position;
    input_state.mouse_screen_position = position;
}
void Input::SetMouseOverUi(bool mouse_over_ui) { input_state.mouse_over_ui = mouse_over_ui; }
glm::vec2 Input::MouseDelta() { return input_state.mouse_delta; }

const std::vector<float>& Input::ScrollEvents() { return input_state.mouse_scroll_events; }
const glm::vec2& Input::MouseScreenPosition() { return input_state.mouse_screen_position; }
bool Input::IsMouseOverUi() { return input_state.mouse_over_ui; }

_SGE_END