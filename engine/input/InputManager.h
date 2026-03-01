#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace atlas::input {

enum class InputAction : uint32_t {
    None = 0,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Crouch,
    Sprint,
    Interact,
    PrimaryAction,
    SecondaryAction
};

enum class InputDevice : uint8_t {
    Keyboard,
    Mouse,
    Gamepad
};

struct InputBinding {
    InputAction action = InputAction::None;
    InputDevice device = InputDevice::Keyboard;
    uint32_t keyCode = 0;
    std::string name;
};

struct InputState {
    bool pressed = false;
    bool held = false;
    bool released = false;
    float value = 0.0f;
};

class InputManager {
public:
    void Init();
    void Shutdown();

    void BindAction(InputAction action, InputDevice device, uint32_t keyCode, const std::string& name);
    void UnbindAction(InputAction action);
    bool HasBinding(InputAction action) const;
    const InputBinding* GetBinding(InputAction action) const;
    size_t BindingCount() const;

    void InjectPress(InputAction action);
    void InjectRelease(InputAction action);
    void InjectAxis(InputAction action, float value);

    InputState GetState(InputAction action) const;
    bool IsPressed(InputAction action) const;
    bool IsHeld(InputAction action) const;
    float GetAxis(InputAction action) const;

    void Update();

    void SetCallback(InputAction action, std::function<void(const InputState&)> callback);

private:
    std::unordered_map<uint32_t, InputBinding> m_bindings;
    std::unordered_map<uint32_t, InputState> m_states;
    std::unordered_map<uint32_t, InputState> m_previousStates;
    std::unordered_map<uint32_t, std::function<void(const InputState&)>> m_callbacks;
    bool m_initialized = false;
};

}
