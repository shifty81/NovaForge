#include "systems/fps_salvage_path_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FPSSalvagePathSystem::FPSSalvagePathSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void FPSSalvagePathSystem::updateComponent(ecs::Entity& /*entity*/, components::FPSSalvagePath& path, float delta_time) {
    if (!path.active) return;

    // Advance cutting progress for any entry points being cut
    for (auto& entry : path.entry_points) {
        if (entry.state == components::FPSSalvagePath::EntryState::Cutting) {
            entry.cut_progress += delta_time;
            if (entry.cut_progress >= entry.cut_required) {
                entry.cut_progress = entry.cut_required;
                entry.state = components::FPSSalvagePath::EntryState::Open;
            }
        }
    }

    // Update exploration progress
    if (path.total_rooms > 0) {
        path.exploration_progress = static_cast<float>(path.rooms_explored) /
                                     static_cast<float>(path.total_rooms);
    }
}

bool FPSSalvagePathSystem::initializePath(const std::string& entity_id,
                                           const std::string& site_id,
                                           const std::string& explorer_id,
                                           int total_rooms) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = getComponentFor(entity_id);
    if (existing) return false;

    auto comp = std::make_unique<components::FPSSalvagePath>();
    comp->site_id = site_id;
    comp->explorer_id = explorer_id;
    comp->total_rooms = total_rooms;
    entity->addComponent(std::move(comp));
    return true;
}

bool FPSSalvagePathSystem::addEntryPoint(const std::string& entity_id,
                                          const std::string& entry_id,
                                          float cut_required,
                                          const std::string& tool_required) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    // Check for duplicate
    if (path->findEntry(entry_id)) return false;

    components::FPSSalvagePath::EntryPoint entry;
    entry.entry_id = entry_id;
    entry.cut_required = cut_required;
    entry.tool_required = tool_required;
    path->entry_points.push_back(entry);
    return true;
}

bool FPSSalvagePathSystem::startCutting(const std::string& entity_id,
                                         const std::string& entry_id) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    auto* entry = path->findEntry(entry_id);
    if (!entry) return false;

    if (entry->state != components::FPSSalvagePath::EntryState::Sealed) return false;

    entry->state = components::FPSSalvagePath::EntryState::Cutting;
    return true;
}

bool FPSSalvagePathSystem::exploreRoom(const std::string& entity_id) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    if (path->rooms_explored >= path->total_rooms) return false;

    path->rooms_explored++;
    if (path->total_rooms > 0) {
        path->exploration_progress = static_cast<float>(path->rooms_explored) /
                                     static_cast<float>(path->total_rooms);
    }
    return true;
}

bool FPSSalvagePathSystem::setActive(const std::string& entity_id, bool active) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    path->active = active;
    return true;
}

bool FPSSalvagePathSystem::addLootNode(const std::string& entity_id,
                                        const std::string& loot_id,
                                        const std::string& item_name,
                                        components::FPSSalvagePath::LootRarity rarity,
                                        float value) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    // Check for duplicate
    if (path->findLoot(loot_id)) return false;

    components::FPSSalvagePath::LootNode node;
    node.loot_id = loot_id;
    node.item_name = item_name;
    node.rarity = rarity;
    node.value = value;
    path->loot_nodes.push_back(node);
    return true;
}

bool FPSSalvagePathSystem::discoverLoot(const std::string& entity_id,
                                         const std::string& loot_id) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    auto* loot = path->findLoot(loot_id);
    if (!loot) return false;
    if (loot->discovered) return false;

    loot->discovered = true;
    return true;
}

bool FPSSalvagePathSystem::collectLoot(const std::string& entity_id,
                                        const std::string& loot_id) {
    auto* path = getComponentFor(entity_id);
    if (!path) return false;

    auto* loot = path->findLoot(loot_id);
    if (!loot) return false;
    if (!loot->discovered) return false;
    if (loot->collected) return false;

    loot->collected = true;
    path->total_collections++;
    return true;
}

float FPSSalvagePathSystem::getExplorationProgress(const std::string& entity_id) const {
    const auto* path = getComponentFor(entity_id);
    if (!path) return 0.0f;

    return path->exploration_progress;
}

int FPSSalvagePathSystem::getDiscoveredLootCount(const std::string& entity_id) const {
    const auto* path = getComponentFor(entity_id);
    if (!path) return 0;

    return path->discoveredCount();
}

int FPSSalvagePathSystem::getCollectedLootCount(const std::string& entity_id) const {
    const auto* path = getComponentFor(entity_id);
    if (!path) return 0;

    return path->collectedCount();
}

std::string FPSSalvagePathSystem::getEntryState(const std::string& entity_id,
                                                  const std::string& entry_id) const {
    const auto* path = getComponentFor(entity_id);
    if (!path) return "unknown";

    const auto* entry = path->findEntry(entry_id);
    if (!entry) return "unknown";

    using ES = components::FPSSalvagePath::EntryState;
    switch (entry->state) {
        case ES::Sealed: return "sealed";
        case ES::Cutting: return "cutting";
        case ES::Open: return "open";
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
