#include "systems/player_session_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using PS = components::PlayerSession;
}

PlayerSessionSystem::PlayerSessionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerSessionSystem::updateComponent(ecs::Entity& entity,
    components::PlayerSession& state, float delta_time) {
    if (!state.active) return;
    if (state.state == PS::SessionState::InGame) {
        state.session_duration += delta_time;
        state.total_play_time += delta_time;
        // Update play time on the selected character slot
        for (auto& slot : state.character_slots) {
            if (slot.character_id == state.selected_character_id && slot.active) {
                slot.play_time += delta_time;
                break;
            }
        }
    }
}

bool PlayerSessionSystem::initialize(const std::string& entity_id,
    const std::string& player_id, const std::string& display_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlayerSession>();
    comp->player_id = player_id;
    comp->display_name = display_name;
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerSessionSystem::connect(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state != PS::SessionState::Disconnected) return false;
    state->state = PS::SessionState::CharacterSelect;
    state->session_start_time = 0.0f;
    state->session_duration = 0.0f;
    state->login_count++;
    return true;
}

bool PlayerSessionSystem::disconnect(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state == PS::SessionState::Disconnected) return false;
    state->state = PS::SessionState::Disconnected;
    state->selected_character_id.clear();
    state->disconnect_count++;
    return true;
}

bool PlayerSessionSystem::reconnect(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state != PS::SessionState::Disconnected) return false;
    state->state = PS::SessionState::Reconnecting;
    state->reconnect_count++;
    return true;
}

bool PlayerSessionSystem::addCharacterSlot(const std::string& entity_id,
    const std::string& character_id, const std::string& character_name,
    const std::string& ship_type, const std::string& location) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->character_slots.size()) >= state->max_character_slots)
        return false;
    for (const auto& slot : state->character_slots) {
        if (slot.character_id == character_id) return false;
    }
    PS::CharacterSlot slot;
    slot.character_id = character_id;
    slot.character_name = character_name;
    slot.ship_type = ship_type;
    slot.location = location;
    slot.active = true;
    state->character_slots.push_back(slot);
    return true;
}

bool PlayerSessionSystem::removeCharacterSlot(const std::string& entity_id,
    const std::string& character_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->character_slots.begin(), state->character_slots.end(),
        [&](const PS::CharacterSlot& s) { return s.character_id == character_id; });
    if (it == state->character_slots.end()) return false;
    state->character_slots.erase(it);
    return true;
}

bool PlayerSessionSystem::selectCharacter(const std::string& entity_id,
    const std::string& character_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state != PS::SessionState::CharacterSelect &&
        state->state != PS::SessionState::Reconnecting) return false;
    for (const auto& slot : state->character_slots) {
        if (slot.character_id == character_id && slot.active) {
            state->selected_character_id = character_id;
            state->state = PS::SessionState::Loading;
            return true;
        }
    }
    return false;
}

bool PlayerSessionSystem::enterGame(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state != PS::SessionState::Loading) return false;
    if (state->selected_character_id.empty()) return false;
    state->state = PS::SessionState::InGame;
    return true;
}

bool PlayerSessionSystem::heartbeat(const std::string& entity_id, float current_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->state == PS::SessionState::Disconnected) return false;
    state->last_heartbeat = current_time;
    return true;
}

int PlayerSessionSystem::getCharacterSlotCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->character_slots.size()) : 0;
}

int PlayerSessionSystem::getSessionState(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->state) : -1;
}

float PlayerSessionSystem::getSessionDuration(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->session_duration : 0.0f;
}

float PlayerSessionSystem::getTotalPlayTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_play_time : 0.0f;
}

int PlayerSessionSystem::getLoginCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->login_count : 0;
}

int PlayerSessionSystem::getDisconnectCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->disconnect_count : 0;
}

int PlayerSessionSystem::getReconnectCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->reconnect_count : 0;
}

std::string PlayerSessionSystem::getSelectedCharacter(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->selected_character_id : "";
}

} // namespace systems
} // namespace atlas
