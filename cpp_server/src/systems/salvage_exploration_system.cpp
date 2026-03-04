#include "systems/salvage_exploration_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

SalvageExplorationSystem::SalvageExplorationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void SalvageExplorationSystem::updateComponent(ecs::Entity& /*entity*/, components::SalvageSite& /*site*/, float /*delta_time*/) {
    // No per-tick behavior needed for salvage sites
}

bool SalvageExplorationSystem::discoverNode(const std::string& site_entity_id) {
    auto* site = getComponentFor(site_entity_id);
    if (!site) return false;
    if (site->discovered_nodes >= site->total_loot_nodes) return false;
    site->discovered_nodes++;
    return true;
}

bool SalvageExplorationSystem::lootNode(const std::string& site_entity_id) {
    auto* site = getComponentFor(site_entity_id);
    if (!site) return false;
    if (site->looted_nodes >= site->discovered_nodes) return false;
    site->looted_nodes++;
    return true;
}

bool SalvageExplorationSystem::isFullyLooted(const std::string& site_entity_id) const {
    const auto* site = getComponentFor(site_entity_id);
    if (!site) return false;
    return site->looted_nodes >= site->total_loot_nodes && site->total_loot_nodes > 0;
}

int SalvageExplorationSystem::getRemainingNodes(const std::string& site_entity_id) const {
    const auto* site = getComponentFor(site_entity_id);
    if (!site) return 0;
    return site->total_loot_nodes - site->looted_nodes;
}

int SalvageExplorationSystem::getDiscoveredNodes(const std::string& site_entity_id) const {
    const auto* site = getComponentFor(site_entity_id);
    if (!site) return 0;
    return site->discovered_nodes;
}

bool SalvageExplorationSystem::hasAncientTech(const std::string& site_entity_id) const {
    const auto* site = getComponentFor(site_entity_id);
    if (!site) return false;
    return site->has_ancient_tech;
}

int SalvageExplorationSystem::generateTrinkets(const std::string& site_entity_id, uint64_t seed) {
    auto* site = getComponentFor(site_entity_id);
    if (!site) return 0;

    // Simple deterministic RNG from seed
    uint64_t state = seed;
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    state = state * 0x2545F4914F6CDD1DULL;

    site->trinket_count = static_cast<int>(state % 6); // 0-5

    // Rare bobblehead: 1/200 chance
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    state = state * 0x2545F4914F6CDD1DULL;
    site->has_rare_bobblehead = (state % 200) == 0;

    return site->trinket_count;
}

} // namespace systems
} // namespace atlas
