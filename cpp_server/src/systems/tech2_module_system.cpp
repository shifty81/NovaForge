#include "systems/tech2_module_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

Tech2ModuleSystem::Tech2ModuleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void Tech2ModuleSystem::updateComponent(ecs::Entity& /*entity*/,
    components::Tech2ModuleState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool Tech2ModuleSystem::initialize(const std::string& entity_id,
    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::Tech2ModuleState>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool Tech2ModuleSystem::registerModule(const std::string& entity_id,
    const std::string& module_id,
    const std::string& base_module_id,
    Category category,
    int meta_level,
    float stat_multiplier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->modules.size() >= static_cast<size_t>(comp->max_modules)) return false;

    // Prevent duplicate registration
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::Tech2ModuleState::ModuleEntry entry;
    entry.module_id = module_id;
    entry.base_module_id = base_module_id;
    entry.category = category;
    entry.meta_level = meta_level;
    entry.stat_multiplier = stat_multiplier;
    comp->modules.push_back(entry);
    return true;
}

bool Tech2ModuleSystem::addLootEntry(const std::string& entity_id,
    const std::string& source_site, const std::string& module_id,
    float drop_chance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    components::Tech2ModuleState::LootTableEntry entry;
    entry.source_site = source_site;
    entry.module_id = module_id;
    entry.drop_chance = std::max(0.0f, std::min(1.0f, drop_chance));
    comp->loot_table.push_back(entry);
    return true;
}

std::string Tech2ModuleSystem::rollLoot(const std::string& entity_id,
    const std::string& source_site, float random_value_0_1) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";

    // Iterate loot table entries for this site, return first that passes
    for (const auto& entry : comp->loot_table) {
        if (entry.source_site == source_site &&
            random_value_0_1 <= entry.drop_chance) {
            return entry.module_id;
        }
    }
    return "";
}

bool Tech2ModuleSystem::acquireModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            m.unlocked = true;
            m.quantity_owned++;
            if (m.category == Category::Tech2) comp->total_tech2_acquired++;
            else if (m.category == Category::Faction) comp->total_faction_acquired++;
            else if (m.category == Category::Deadspace) comp->total_deadspace_acquired++;
            return true;
        }
    }
    return false;
}

int Tech2ModuleSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

int Tech2ModuleSystem::getOwnedCount(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.quantity_owned;
    }
    return 0;
}

int Tech2ModuleSystem::getTotalTech2Acquired(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_tech2_acquired : 0;
}

int Tech2ModuleSystem::getTotalFactionAcquired(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_faction_acquired : 0;
}

int Tech2ModuleSystem::getTotalDeadspaceAcquired(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_deadspace_acquired : 0;
}

} // namespace systems
} // namespace atlas
