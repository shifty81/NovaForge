#include "systems/session_persistence_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

SessionPersistenceSystem::SessionPersistenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SessionPersistenceSystem::updateComponent(ecs::Entity& /*entity*/,
    components::SessionPersistenceState& state, float delta_time) {
    if (!state.active) return;

    state.time_since_last_save += delta_time;

    // Auto-save when interval elapses and state is dirty
    if (state.status == components::SessionPersistenceState::SaveStatus::Dirty &&
        state.time_since_last_save >= state.auto_save_interval) {
        state.status = components::SessionPersistenceState::SaveStatus::Saving;
        // Simulate save completion (in real code this would be async I/O)
        state.total_saves++;
        state.time_since_last_save = 0.0f;
        state.status = components::SessionPersistenceState::SaveStatus::Clean;
    }
}

bool SessionPersistenceSystem::markDirty(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->status != components::SessionPersistenceState::SaveStatus::Saving) {
        state->status = components::SessionPersistenceState::SaveStatus::Dirty;
    }
    return true;
}

bool SessionPersistenceSystem::triggerSave(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    state->status = components::SessionPersistenceState::SaveStatus::Saving;
    // Simulate save
    state->total_saves++;
    state->time_since_last_save = 0.0f;
    state->status = components::SessionPersistenceState::SaveStatus::Clean;
    return true;
}

bool SessionPersistenceSystem::triggerLoad(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    state->total_loads++;
    state->time_since_last_save = 0.0f;
    state->status = components::SessionPersistenceState::SaveStatus::Clean;
    return true;
}

int SessionPersistenceSystem::getTotalSaves(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_saves : 0;
}

int SessionPersistenceSystem::getTotalLoads(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_loads : 0;
}

bool SessionPersistenceSystem::isDirty(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->status == components::SessionPersistenceState::SaveStatus::Dirty;
}

float SessionPersistenceSystem::getTimeSinceLastSave(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->time_since_last_save : 0.0f;
}

std::string SessionPersistenceSystem::getCurrentSystem(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->current_system : "";
}

} // namespace systems
} // namespace atlas
