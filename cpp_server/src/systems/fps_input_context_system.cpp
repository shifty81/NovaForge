#include "systems/fps_input_context_system.h"
#include <iostream>

namespace atlas {
namespace systems {

FPSInputContextSystem::FPSInputContextSystem(ecs::World* world)
    : SingleComponentSystem<components::FPSCharacterState>(world)
{
}

void FPSInputContextSystem::updateComponent(ecs::Entity& /*entity*/,
                                             components::FPSCharacterState& /*state*/,
                                             float /*delta_time*/)
{
    // Context tracking is stateless per-tick; nothing to do here.
}

bool FPSInputContextSystem::setContext(const std::string& player_id, InputContext context) {
    auto it = m_contexts.find(player_id);
    if (it == m_contexts.end()) return false;
    it->second = context;
    return true;
}

FPSInputContextSystem::InputContext
FPSInputContextSystem::getContext(const std::string& player_id) const {
    auto it = m_contexts.find(player_id);
    if (it != m_contexts.end()) return it->second;
    return InputContext::Tactical;  // Default
}

bool FPSInputContextSystem::registerPlayer(const std::string& player_id) {
    if (m_contexts.count(player_id)) return false;
    m_contexts[player_id] = InputContext::Tactical;
    return true;
}

bool FPSInputContextSystem::unregisterPlayer(const std::string& player_id) {
    return m_contexts.erase(player_id) > 0;
}

bool FPSInputContextSystem::isInFPSContext(const std::string& player_id) const {
    return getContext(player_id) == InputContext::FPS;
}

bool FPSInputContextSystem::isInTacticalContext(const std::string& player_id) const {
    return getContext(player_id) == InputContext::Tactical;
}

bool FPSInputContextSystem::isInCockpitContext(const std::string& player_id) const {
    return getContext(player_id) == InputContext::Cockpit;
}

FPSInputContextSystem::FPSAction
FPSInputContextSystem::getFPSActionForKey(int key_code) const {
    switch (key_code) {
        case KEY_W:      return FPSAction::MoveForward;
        case KEY_S:      return FPSAction::MoveBackward;
        case KEY_A:      return FPSAction::MoveLeft;
        case KEY_D:      return FPSAction::MoveRight;
        case KEY_SPACE:  return FPSAction::Jump;
        case KEY_LSHIFT: return FPSAction::Sprint;
        case KEY_C:      return FPSAction::Crouch;
        case KEY_LCTRL:  return FPSAction::Crouch;
        case KEY_E:      return FPSAction::Interact;
        case KEY_R:      return FPSAction::Reload;
        case KEY_F:      return FPSAction::ToggleFlashlight;
        case KEY_TAB:    return FPSAction::Inventory;
        case KEY_V:      return FPSAction::ToggleViewMode;
        default:         return FPSAction::None;
    }
}

bool FPSInputContextSystem::shouldConsumeKey(const std::string& player_id, int key_code) const {
    auto ctx = getContext(player_id);
    if (ctx == InputContext::FPS) {
        // In FPS mode, WASD/Space/Shift/C/E/R/F/Tab/V are consumed
        return getFPSActionForKey(key_code) != FPSAction::None;
    }
    // In tactical or cockpit mode, these keys pass through to their handlers
    return false;
}

bool FPSInputContextSystem::shouldCaptureMouse(const std::string& player_id) const {
    auto ctx = getContext(player_id);
    // FPS mode captures the mouse for free look
    return ctx == InputContext::FPS;
}

bool FPSInputContextSystem::isFreeLookActive(const std::string& player_id) const {
    auto ctx = getContext(player_id);
    // Free look (no button hold required) in FPS mode
    return ctx == InputContext::FPS;
}

std::string FPSInputContextSystem::contextName(InputContext ctx) {
    switch (ctx) {
        case InputContext::Tactical: return "Tactical";
        case InputContext::FPS:      return "FPS";
        case InputContext::Cockpit:  return "Cockpit";
    }
    return "Unknown";
}

std::string FPSInputContextSystem::actionName(FPSAction action) {
    switch (action) {
        case FPSAction::None:             return "None";
        case FPSAction::MoveForward:      return "MoveForward";
        case FPSAction::MoveBackward:     return "MoveBackward";
        case FPSAction::MoveLeft:         return "MoveLeft";
        case FPSAction::MoveRight:        return "MoveRight";
        case FPSAction::Jump:             return "Jump";
        case FPSAction::Crouch:           return "Crouch";
        case FPSAction::Sprint:           return "Sprint";
        case FPSAction::Interact:         return "Interact";
        case FPSAction::Reload:           return "Reload";
        case FPSAction::ToggleFlashlight: return "ToggleFlashlight";
        case FPSAction::Inventory:        return "Inventory";
        case FPSAction::PrimaryFire:      return "PrimaryFire";
        case FPSAction::SecondaryFire:    return "SecondaryFire";
        case FPSAction::BoardShip:        return "BoardShip";
        case FPSAction::ExitToStation:    return "ExitToStation";
        case FPSAction::ToggleViewMode:   return "ToggleViewMode";
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
