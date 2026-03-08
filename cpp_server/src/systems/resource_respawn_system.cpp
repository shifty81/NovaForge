#include "systems/resource_respawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ResourceRespawnSystem::ResourceRespawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ResourceRespawnSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ResourceRespawnState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed += delta_time;

    // Tick down cooldowns on depleted resources
    for (auto& r : state.entries) {
        if (r.depleted && !r.respawned) {
            r.cooldown_remaining -= delta_time;
            if (r.cooldown_remaining <= 0.0f) {
                r.cooldown_remaining = 0.0f;
                r.depleted = false;
                r.respawned = true;
                state.total_respawns++;
            }
        }
    }
}

bool ResourceRespawnSystem::initialize(const std::string& entity_id,
    const std::string& zone_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (zone_id.empty()) return false;
    auto comp = std::make_unique<components::ResourceRespawnState>();
    comp->zone_id = zone_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ResourceRespawnSystem::addResource(const std::string& entity_id,
    const std::string& resource_id, const std::string& resource_type, float cooldown) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (resource_id.empty() || resource_type.empty()) return false;
    if (cooldown <= 0.0f) return false;
    if (static_cast<int>(state->entries.size()) >= state->max_entries) return false;
    for (const auto& r : state->entries) {
        if (r.resource_id == resource_id) return false;
    }
    components::ResourceRespawnState::RespawnEntry entry;
    entry.resource_id = resource_id;
    entry.resource_type = resource_type;
    entry.cooldown_total = cooldown;
    entry.cooldown_remaining = 0.0f;
    state->entries.push_back(entry);
    return true;
}

bool ResourceRespawnSystem::removeResource(const std::string& entity_id,
    const std::string& resource_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->entries.begin(), state->entries.end(),
        [&](const components::ResourceRespawnState::RespawnEntry& r) {
            return r.resource_id == resource_id;
        });
    if (it == state->entries.end()) return false;
    state->entries.erase(it);
    return true;
}

bool ResourceRespawnSystem::depleteResource(const std::string& entity_id,
    const std::string& resource_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& r : state->entries) {
        if (r.resource_id == resource_id) {
            if (r.depleted) return false; // already depleted
            r.depleted = true;
            r.respawned = false;
            r.cooldown_remaining = r.cooldown_total;
            state->total_depletions++;
            return true;
        }
    }
    return false;
}

bool ResourceRespawnSystem::setYieldMultiplier(const std::string& entity_id,
    const std::string& resource_id, float multiplier) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (multiplier <= 0.0f) return false;
    for (auto& r : state->entries) {
        if (r.resource_id == resource_id) {
            r.yield_multiplier = multiplier;
            return true;
        }
    }
    return false;
}

int ResourceRespawnSystem::getResourceCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->entries.size()) : 0;
}

int ResourceRespawnSystem::getDepletedCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& r : state->entries) {
        if (r.depleted) count++;
    }
    return count;
}

int ResourceRespawnSystem::getRespawnedCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& r : state->entries) {
        if (r.respawned) count++;
    }
    return count;
}

int ResourceRespawnSystem::getTotalRespawns(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_respawns : 0;
}

int ResourceRespawnSystem::getTotalDepletions(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_depletions : 0;
}

float ResourceRespawnSystem::getCooldownRemaining(const std::string& entity_id,
    const std::string& resource_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& r : state->entries) {
        if (r.resource_id == resource_id) return r.cooldown_remaining;
    }
    return 0.0f;
}

bool ResourceRespawnSystem::isResourceDepleted(const std::string& entity_id,
    const std::string& resource_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& r : state->entries) {
        if (r.resource_id == resource_id) return r.depleted;
    }
    return false;
}

bool ResourceRespawnSystem::isResourceRespawned(const std::string& entity_id,
    const std::string& resource_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& r : state->entries) {
        if (r.resource_id == resource_id) return r.respawned;
    }
    return false;
}

std::string ResourceRespawnSystem::getZoneId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->zone_id : "";
}

} // namespace systems
} // namespace atlas
