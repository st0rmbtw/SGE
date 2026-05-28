#include <unordered_set>
#include <SGE/input.hpp>

static struct InputState {
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

void sge::Input::Press(sge::Key key, uint8_t modifiers) {
    const sge::KeyWithModifiers value = sge::KeyWithModifiers(key, modifiers);
    if (input_state.keyboard_pressed.insert(value).second) {
        input_state.keyboard_just_pressed.insert(value);
    }
}

void sge::Input::Release(sge::Key key, uint8_t modifiers) {
    const sge::KeyWithModifiers value = sge::KeyWithModifiers(key, modifiers);
    if (input_state.keyboard_pressed.erase(value) > 0) {
        input_state.keyboard_just_released.insert(value);
    }
}

void sge::Input::Clear() {
    input_state.keyboard_just_pressed.clear();
    input_state.keyboard_just_released.clear();

    input_state.mouse_just_pressed.clear();
    input_state.mouse_just_released.clear();
    input_state.mouse_scroll_events.clear();
    input_state.mouse_delta = glm::vec2(0.0f);

    input_state.codepoint_queue.clear();
}

void sge::Input::PushCodePoint(uint32_t codepoint) {
    input_state.codepoint_queue.push_back(codepoint);
}

void sge::Input::Press(MouseButton button) {
    if (input_state.mouse_pressed.insert(button).second) {
        input_state.mouse_just_pressed.insert(button);
    }
}

void sge::Input::Release(MouseButton button) {
    if (input_state.mouse_pressed.erase(button) > 0) {
        input_state.mouse_just_released.insert(button);
    }
}

void sge::Input::PushMouseScrollEvent(float y) noexcept {
    input_state.mouse_scroll_events.push_back(y);
}

void sge::Input::SetCursorPosition(glm::vec2 position) noexcept {    
    input_state.cursor_position = position;
}

void sge::Input::SetMouseDelta(glm::vec2 delta) noexcept {
    input_state.mouse_delta = delta;
}


// ----------------- Public -----------------


bool sge::Input::Pressed(Key key) {
    return input_state.keyboard_pressed.find(key) != input_state.keyboard_pressed.end();
}

bool sge::Input::JustPressed(Key key) {
    return input_state.keyboard_just_pressed.find(key) != input_state.keyboard_just_pressed.end();
}

bool sge::Input::Pressed(Key key, uint8_t modifiers) {
    const auto entry = input_state.keyboard_pressed.find(key);
    if (entry == input_state.keyboard_pressed.end()) return false;
    return entry->modifiers == modifiers;
}

bool sge::Input::JustPressed(Key key, uint8_t modifiers) {
    const auto entry = input_state.keyboard_just_pressed.find(key);
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