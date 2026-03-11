#include "systems/asset_safety_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AssetSafetySystem::AssetSafetySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void AssetSafetySystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::AssetSafetyState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    for (auto& entry : comp.entries) {
        if (!entry.claimed && !entry.expired) {
            entry.expires_in -= delta_time;
            if (entry.expires_in <= 0.0f) {
                entry.expires_in = 0.0f;
                entry.expired    = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool AssetSafetySystem::initialize(const std::string& entity_id,
                                    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || owner_id.empty()) return false;
    auto comp = std::make_unique<components::AssetSafetyState>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Safety entry management
// ---------------------------------------------------------------------------

bool AssetSafetySystem::triggerSafety(const std::string& entity_id,
                                       const std::string& structure_id,
                                       const std::string& structure_name,
                                       const std::string& asset_id,
                                       const std::string& asset_name,
                                       int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (structure_id.empty() || structure_name.empty()) return false;
    if (asset_id.empty() || asset_name.empty()) return false;
    if (quantity <= 0) return false;
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) return false;

    for (const auto& e : comp->entries) {
        if (e.asset_id == asset_id) return false;
    }

    components::AssetSafetyState::AssetEntry entry;
    entry.structure_id   = structure_id;
    entry.structure_name = structure_name;
    entry.asset_id       = asset_id;
    entry.asset_name     = asset_name;
    entry.quantity       = quantity;
    entry.triggered_at   = comp->elapsed;
    entry.expires_in     = comp->safety_duration;
    comp->entries.push_back(entry);
    comp->total_triggered++;
    return true;
}

bool AssetSafetySystem::claimAsset(const std::string& entity_id,
                                    const std::string& asset_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.asset_id == asset_id) {
            if (e.claimed || e.expired) return false;
            e.claimed = true;
            comp->total_claimed++;
            return true;
        }
    }
    return false;
}

bool AssetSafetySystem::claimAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    bool any = false;
    for (auto& e : comp->entries) {
        if (!e.claimed && !e.expired) {
            e.claimed = true;
            comp->total_claimed++;
            any = true;
        }
    }
    return any;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int AssetSafetySystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

int AssetSafetySystem::getUnclaimedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (!e.claimed && !e.expired) count++;
    }
    return count;
}

int AssetSafetySystem::getExpiredCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.expired) count++;
    }
    return count;
}

int AssetSafetySystem::getTotalTriggered(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_triggered : 0;
}

int AssetSafetySystem::getTotalClaimed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_claimed : 0;
}

bool AssetSafetySystem::hasEntry(const std::string& entity_id,
                                  const std::string& asset_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.asset_id == asset_id) return true;
    }
    return false;
}

std::string AssetSafetySystem::getOwner(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->owner_id : std::string();
}

} // namespace systems
} // namespace atlas
