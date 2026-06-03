#include <SGE/input.hpp>
#include <unordered_set>

namespace {

struct InputState {
    std::unordered_set<sge::KeyWithModifiers> keyboard_pressed;
    std::unordered_set<sge::KeyWithModifiers> keyboard_just_pressed;
    std::unordered_set<sge::KeyWithModifiers> keyboard_just_released;

    std::unordered_set<sge::MouseButton> mouse_pressed;
    std::unordered_set<sge::MouseButton> mouse_just_pressed;
    std::unordered_set<sge::MouseButton> mouse_just_released;
    std::vector<float> mouse_scroll_events;
    std::vector<uint32_t> codepoint_queue;
    glm::vec2 cursor_position;
    glm::vec2 mouse_delta;
} input_state;

} // namespace

void sge::Input::Clear() {
    input_state.keyboard_just_pressed.clear();
    input_state.keyboard_just_released.clear();

    input_state.mouse_just_pressed.clear();
    input_state.mouse_just_released.clear();
    input_state.mouse_scroll_events.clear();
    input_state.mouse_delta = glm::vec2(0.0f);

    input_state.codepoint_queue.clear();
}

void sge::Input::ProcessEvent(const InputEvent& inputEvent) {
    switch (inputEvent.Type) {
    case InputEventType::Key: {
        KeyInputEvent event = inputEvent.KeyEvent;
        if (event.Pressed) {
            const auto value = sge::KeyWithModifiers{.key = event.Key, .modifiers = static_cast<uint8_t>(event.Mods)};
            if (input_state.keyboard_pressed.insert(value).second) {
                input_state.keyboard_just_pressed.insert(value);
            }
        } else {
            const auto value = sge::KeyWithModifiers{.key = event.Key, .modifiers = static_cast<uint8_t>(event.Mods)};
            if (input_state.keyboard_pressed.erase(value) > 0) {
                input_state.keyboard_just_released.insert(value);
            }
        }
    }
    break;
    case InputEventType::MouseButton: {
        MouseButtonInputEvent event = inputEvent.MouseButtonEvent;
        if (event.Pressed) {
            if (input_state.mouse_pressed.insert(event.Button).second) {
                input_state.mouse_just_pressed.insert(event.Button);
            }
        } else {
            if (input_state.mouse_pressed.erase(event.Button) > 0) {
                input_state.mouse_just_released.insert(event.Button);
            }
        }
    }
    break;
    case InputEventType::CursorMove: {
        const glm::vec2 newPosition = inputEvent.CursorMoveEvent.Pos;
        const glm::vec2 delta = input_state.cursor_position - newPosition;

        input_state.mouse_delta += delta;
        input_state.cursor_position = newPosition;
    }
    break;
    case InputEventType::Scroll: {
        input_state.mouse_scroll_events.push_back(inputEvent.ScrollEvent.ScrollY);
    }
    break;
    case InputEventType::CodePoint: {
        input_state.codepoint_queue.push_back(inputEvent.CodePointEvent.CodePoint);
    }
    break;
    }
}

bool sge::Input::Pressed(Key key) {
    const auto key_with_mods = KeyWithModifiers{ .key = key };
    return input_state.keyboard_pressed.find(key_with_mods) != input_state.keyboard_pressed.end();
}

bool sge::Input::JustPressed(Key key) {
    const auto key_with_mods = KeyWithModifiers{ .key = key };
    return input_state.keyboard_just_pressed.find(key_with_mods) != input_state.keyboard_just_pressed.end();
}

bool sge::Input::Pressed(Key key, uint8_t modifiers) {
    const auto key_with_mods = KeyWithModifiers{ .key = key };
    const auto entry = input_state.keyboard_pressed.find(key_with_mods);
    if (entry == input_state.keyboard_pressed.end()) return false;
    return entry->modifiers == modifiers;
}

bool sge::Input::JustPressed(Key key, uint8_t modifiers) {
    const auto key_with_mods = KeyWithModifiers{ .key = key };
    const auto entry = input_state.keyboard_just_pressed.find(key_with_mods);
    if (entry == input_state.keyboard_just_pressed.end()) return false;
    return entry->modifiers == modifiers;
}

bool sge::Input::Pressed(MouseButton button) {
    return input_state.mouse_pressed.find(button) != input_state.mouse_pressed.end();
}

bool sge::Input::JustPressed(MouseButton button) {
    return input_state.mouse_just_pressed.find(button) != input_state.mouse_just_pressed.end();
}

bool sge::Input::JustReleased(MouseButton button) {
    return input_state.mouse_just_released.find(button) != input_state.mouse_just_released.end();
}

glm::vec2 sge::Input::MouseDelta() noexcept {
    return input_state.mouse_delta;
}

const std::vector<float>& sge::Input::ScrollEvents() noexcept {
    return input_state.mouse_scroll_events;
}

const glm::vec2& sge::Input::CursorPosition() noexcept {
    return input_state.cursor_position;
}

const std::vector<uint32_t>& sge::Input::CodePoints() noexcept {
    return input_state.codepoint_queue;
}

const std::unordered_set<sge::KeyWithModifiers>& sge::Input::GetJustPressedKeys() noexcept {
    return input_state.keyboard_just_pressed;
}

const std::unordered_set<sge::KeyWithModifiers>& sge::Input::GetJustReleasedKeys() noexcept {
    return input_state.keyboard_just_released;
}