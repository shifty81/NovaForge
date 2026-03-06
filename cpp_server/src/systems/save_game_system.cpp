#include "systems/save_game_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SGS = components::SaveGameState;
}

SaveGameSystem::SaveGameSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SaveGameSystem::updateComponent(ecs::Entity& entity,
    components::SaveGameState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
    // Auto-save timer: tick down and trigger save when dirty
    if (state.status == SGS::SaveStatus::Idle) {
        state.auto_save_timer += delta_time;
    }
}

bool SaveGameSystem::initialize(const std::string& entity_id,
    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SaveGameState>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SaveGameSystem::createSaveSlot(const std::string& entity_id,
    const std::string& slot_id, const std::string& character_name,
    const std::string& location, const std::string& ship_type,
    double wallet_balance, int skill_points, float play_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->slots.size()) >= state->max_slots) return false;
    for (const auto& s : state->slots) {
        if (s.slot_id == slot_id) return false;
    }
    SGS::SaveSlot slot;
    slot.slot_id = slot_id;
    slot.character_name = character_name;
    slot.location = location;
    slot.ship_type = ship_type;
    slot.wallet_balance = wallet_balance;
    slot.skill_points = skill_points;
    slot.play_time = play_time;
    slot.occupied = true;
    state->slots.push_back(slot);
    return true;
}

bool SaveGameSystem::deleteSaveSlot(const std::string& entity_id,
    const std::string& slot_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->slots.begin(), state->slots.end(),
        [&](const SGS::SaveSlot& s) { return s.slot_id == slot_id; });
    if (it == state->slots.end()) return false;
    state->slots.erase(it);
    return true;
}

bool SaveGameSystem::overwriteSaveSlot(const std::string& entity_id,
    const std::string& slot_id, const std::string& character_name,
    const std::string& location, const std::string& ship_type,
    double wallet_balance, int skill_points, float play_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& s : state->slots) {
        if (s.slot_id == slot_id) {
            s.character_name = character_name;
            s.location = location;
            s.ship_type = ship_type;
            s.wallet_balance = wallet_balance;
            s.skill_points = skill_points;
            s.play_time = play_time;
            return true;
        }
    }
    return false;
}

bool SaveGameSystem::loadSaveSlot(const std::string& entity_id,
    const std::string& slot_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& s : state->slots) {
        if (s.slot_id == slot_id) {
            if (s.corrupted) return false;
            return true;
        }
    }
    return false;
}

bool SaveGameSystem::markDirty(const std::string& entity_id,
    const std::string& category) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (category == "ship") state->ship_dirty = true;
    else if (category == "wallet") state->wallet_dirty = true;
    else if (category == "skills") state->skills_dirty = true;
    else if (category == "standings") state->standings_dirty = true;
    else if (category == "cargo") state->cargo_dirty = true;
    else if (category == "missions") state->missions_dirty = true;
    else return false;
    return true;
}

bool SaveGameSystem::clearDirty(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->ship_dirty = false;
    state->wallet_dirty = false;
    state->skills_dirty = false;
    state->standings_dirty = false;
    state->cargo_dirty = false;
    state->missions_dirty = false;
    return true;
}

bool SaveGameSystem::isDirty(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->ship_dirty || state->wallet_dirty || state->skills_dirty ||
           state->standings_dirty || state->cargo_dirty || state->missions_dirty;
}

bool SaveGameSystem::beginSave(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->status != SGS::SaveStatus::Idle) return false;
    state->status = SGS::SaveStatus::Saving;
    return true;
}

bool SaveGameSystem::completeSave(const std::string& entity_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->status != SGS::SaveStatus::Saving) return false;
    state->status = SGS::SaveStatus::Idle;
    state->total_saves++;
    state->last_save_time = timestamp;
    state->auto_save_timer = 0.0f;
    // Mark slots with save timestamp
    for (auto& s : state->slots) {
        if (s.occupied) s.save_timestamp = timestamp;
    }
    return true;
}

bool SaveGameSystem::beginLoad(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->status != SGS::SaveStatus::Idle) return false;
    state->status = SGS::SaveStatus::Loading;
    return true;
}

bool SaveGameSystem::completeLoad(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->status != SGS::SaveStatus::Loading) return false;
    state->status = SGS::SaveStatus::Idle;
    state->total_loads++;
    return true;
}

bool SaveGameSystem::reportError(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->status = SGS::SaveStatus::Error;
    state->save_errors++;
    return true;
}

int SaveGameSystem::getSlotCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->slots.size()) : 0;
}

int SaveGameSystem::getOccupiedSlotCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& s : state->slots) {
        if (s.occupied) count++;
    }
    return count;
}

int SaveGameSystem::getSaveStatus(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->status) : -1;
}

int SaveGameSystem::getTotalSaves(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_saves : 0;
}

int SaveGameSystem::getTotalLoads(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_loads : 0;
}

int SaveGameSystem::getSaveErrors(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->save_errors : 0;
}

float SaveGameSystem::getLastSaveTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->last_save_time : 0.0f;
}

} // namespace systems
} // namespace atlas
