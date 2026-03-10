#include "systems/relay_clone_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

RelayCloneSystem::RelayCloneSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void RelayCloneSystem::updateComponent(ecs::Entity& /*entity*/,
    components::RelayCloneState& state, float delta_time) {
    if (!state.active) return;

    state.elapsed += delta_time;

    // Tick down cooldown
    if (state.cooldown_remaining > 0.0f) {
        state.cooldown_remaining = (std::max)(0.0f, state.cooldown_remaining - delta_time);
    }
}

bool RelayCloneSystem::initialize(const std::string& entity_id,
    const std::string& character_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RelayCloneState>();
    comp->character_id = character_id.empty() ? entity_id : character_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool RelayCloneSystem::installClone(const std::string& entity_id,
    const std::string& clone_id, const std::string& station_id,
    const std::string& station_name) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;
    if (static_cast<int>(state->clones.size()) >= state->max_clones) return false;

    // No duplicate clone IDs
    for (const auto& c : state->clones) {
        if (c.clone_id == clone_id) return false;
    }

    components::RelayCloneState::CloneEntry entry;
    entry.clone_id = clone_id;
    entry.station_id = station_id;
    entry.station_name = station_name;
    entry.install_time = state->elapsed;
    state->clones.push_back(entry);
    return true;
}

bool RelayCloneSystem::destroyClone(const std::string& entity_id,
    const std::string& clone_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->clones.begin(), state->clones.end(),
        [&](const components::RelayCloneState::CloneEntry& c) {
            return c.clone_id == clone_id;
        });
    if (it == state->clones.end()) return false;
    state->clones.erase(it);
    return true;
}

bool RelayCloneSystem::jumpToClone(const std::string& entity_id,
    const std::string& clone_id) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;
    if (state->cooldown_remaining > 0.0f) return false;

    // Verify clone exists
    bool found = false;
    for (const auto& c : state->clones) {
        if (c.clone_id == clone_id) { found = true; break; }
    }
    if (!found) return false;

    state->cooldown_remaining = state->jump_cooldown;
    state->total_jumps++;
    return true;
}

bool RelayCloneSystem::addImplant(const std::string& entity_id,
    const std::string& clone_id, const std::string& implant_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& c : state->clones) {
        if (c.clone_id == clone_id) {
            // No duplicate implants
            for (const auto& imp : c.implants) {
                if (imp == implant_id) return false;
            }
            c.implants.push_back(implant_id);
            return true;
        }
    }
    return false;
}

bool RelayCloneSystem::removeImplant(const std::string& entity_id,
    const std::string& clone_id, const std::string& implant_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& c : state->clones) {
        if (c.clone_id == clone_id) {
            auto it = std::find(c.implants.begin(), c.implants.end(), implant_id);
            if (it == c.implants.end()) return false;
            c.implants.erase(it);
            return true;
        }
    }
    return false;
}

int RelayCloneSystem::getCloneCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->clones.size()) : 0;
}

int RelayCloneSystem::getTotalJumps(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_jumps : 0;
}

float RelayCloneSystem::getCooldownRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->cooldown_remaining : 0.0f;
}

bool RelayCloneSystem::isOnCooldown(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->cooldown_remaining > 0.0f) : false;
}

bool RelayCloneSystem::setMaxClones(const std::string& entity_id, int max_clones) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->max_clones = (std::max)(1, max_clones);
    return true;
}

int RelayCloneSystem::getMaxClones(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->max_clones : 0;
}

} // namespace systems
} // namespace atlas
