#include "systems/remote_repair_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RemoteRepairSystem::RemoteRepairSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void RemoteRepairSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::RemoteRepairState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    for (auto& mod : comp.modules) {
        if (!mod.active) continue;
        mod.cycle_elapsed += delta_time;
        if (mod.cycle_elapsed >= mod.cycle_time) {
            mod.cycle_elapsed -= mod.cycle_time;
            mod.total_reps++;
            comp.total_cycles++;
            switch (mod.type) {
                case components::RemoteRepairState::RepairType::Shield:
                    comp.total_shield_repaired += mod.rep_amount;
                    break;
                case components::RemoteRepairState::RepairType::Armor:
                    comp.total_armor_repaired += mod.rep_amount;
                    break;
                case components::RemoteRepairState::RepairType::Hull:
                    comp.total_hull_repaired += mod.rep_amount;
                    break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool RemoteRepairSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RemoteRepairState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Module management
// ---------------------------------------------------------------------------

bool RemoteRepairSystem::addModule(
        const std::string& entity_id,
        const std::string& module_id,
        components::RemoteRepairState::RepairType type,
        float rep_amount,
        float optimal_range,
        float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty()) return false;
    if (rep_amount <= 0.0f) return false;
    if (optimal_range <= 0.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::RemoteRepairState::RepairModule mod;
    mod.module_id     = module_id;
    mod.type          = type;
    mod.rep_amount    = rep_amount;
    mod.optimal_range = optimal_range;
    mod.cycle_time    = cycle_time;
    comp->modules.push_back(mod);
    return true;
}

bool RemoteRepairSystem::removeModule(const std::string& entity_id,
                                       const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::RemoteRepairState::RepairModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    return true;
}

// ---------------------------------------------------------------------------
// Activation
// ---------------------------------------------------------------------------

bool RemoteRepairSystem::activateModule(const std::string& entity_id,
                                         const std::string& module_id,
                                         const std::string& target_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_id.empty()) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::RemoteRepairState::RepairModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    if (it->active) return false;  // already active
    it->active    = true;
    it->target_id = target_id;
    it->cycle_elapsed = 0.0f;
    return true;
}

bool RemoteRepairSystem::deactivateModule(const std::string& entity_id,
                                           const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::RemoteRepairState::RepairModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    if (!it->active) return false;  // already inactive
    it->active    = false;
    it->target_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool RemoteRepairSystem::setRepAmount(const std::string& entity_id,
                                       const std::string& module_id,
                                       float rep_amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rep_amount <= 0.0f) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::RemoteRepairState::RepairModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    it->rep_amount = rep_amount;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int RemoteRepairSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

int RemoteRepairSystem::getActiveModuleCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->modules) {
        if (m.active) count++;
    }
    return count;
}

bool RemoteRepairSystem::isModuleActive(const std::string& entity_id,
                                         const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.active;
    }
    return false;
}

float RemoteRepairSystem::getTotalShieldRepaired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_shield_repaired : 0.0f;
}

float RemoteRepairSystem::getTotalArmorRepaired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_armor_repaired : 0.0f;
}

float RemoteRepairSystem::getTotalHullRepaired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_hull_repaired : 0.0f;
}

int RemoteRepairSystem::getTotalCycles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles : 0;
}

std::string RemoteRepairSystem::getTargetId(
        const std::string& entity_id,
        const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.target_id;
    }
    return "";
}

int RemoteRepairSystem::getModuleReps(const std::string& entity_id,
                                       const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.total_reps;
    }
    return 0;
}

} // namespace systems
} // namespace atlas
