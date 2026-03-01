#include "InputManager.h"

namespace atlas::input {

void InputManager::Init() {
    m_bindings.clear();
    m_states.clear();
    m_previousStates.clear();
    m_callbacks.clear();
    m_initialized = true;
}

void InputManager::Shutdown() {
    m_bindings.clear();
    m_states.clear();
    m_previousStates.clear();
    m_callbacks.clear();
    m_initialized = false;
}

void InputManager::BindAction(InputAction action, InputDevice device, uint32_t keyCode, const std::string& name) {
    auto key = static_cast<uint32_t>(action);
    InputBinding binding;
    binding.action = action;
    binding.device = device;
    binding.keyCode = keyCode;
    binding.name = name;
    m_bindings[key] = binding;
    m_states[key] = InputState{};
    m_previousStates[key] = InputState{};
}

void InputManager::UnbindAction(InputAction action) {
    auto key = static_cast<uint32_t>(action);
    m_bindings.erase(key);
    m_states.erase(key);
    m_previousStates.erase(key);
    m_callbacks.erase(key);
}

bool InputManager::HasBinding(InputAction action) const {
    return m_bindings.count(static_cast<uint32_t>(action)) > 0;
}

const InputBinding* InputManager::GetBinding(InputAction action) const {
    auto it = m_bindings.find(static_cast<uint32_t>(action));
    if (it != m_bindings.end()) {
        return &it->second;
    }
    return nullptr;
}

size_t InputManager::BindingCount() const {
    return m_bindings.size();
}

void InputManager::InjectPress(InputAction action) {
    auto key = static_cast<uint32_t>(action);
    if (m_states.count(key)) {
        m_states[key].pressed = true;
        m_states[key].held = true;
        m_states[key].released = false;
        m_states[key].value = 1.0f;
    }
}

void InputManager::InjectRelease(InputAction action) {
    auto key = static_cast<uint32_t>(action);
    if (m_states.count(key)) {
        m_states[key].pressed = false;
        m_states[key].held = false;
        m_states[key].released = true;
        m_states[key].value = 0.0f;
    }
}

void InputManager::InjectAxis(InputAction action, float value) {
    auto key = static_cast<uint32_t>(action);
    if (m_states.count(key)) {
        m_states[key].value = value;
        m_states[key].held = (value != 0.0f);
    }
}

InputState InputManager::GetState(InputAction action) const {
    auto it = m_states.find(static_cast<uint32_t>(action));
    if (it != m_states.end()) {
        return it->second;
    }
    return InputState{};
}

bool InputManager::IsPressed(InputAction action) const {
    return GetState(action).pressed;
}

bool InputManager::IsHeld(InputAction action) const {
    return GetState(action).held;
}

float InputManager::GetAxis(InputAction action) const {
    return GetState(action).value;
}

void InputManager::Update() {
    for (auto& [key, state] : m_states) {
        // Fire callbacks
        auto cbIt = m_callbacks.find(key);
        if (cbIt != m_callbacks.end() && cbIt->second) {
            if (state.pressed || state.released) {
                cbIt->second(state);
            }
        }
        // Transition: pressed becomes held, released becomes idle
        m_previousStates[key] = state;
        state.pressed = false;
        state.released = false;
    }
}

void InputManager::SetCallback(InputAction action, std::function<void(const InputState&)> callback) {
    m_callbacks[static_cast<uint32_t>(action)] = callback;
}

}
