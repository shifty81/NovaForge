#include "systems/module_overheat_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ModuleOverheatSystem::ModuleOverheatSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ModuleOverheatSystem::updateComponent(ecs::Entity& entity,
    components::ModuleOverheat& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& mod : comp.modules) {
        if (mod.burned_out) continue;

        // Passive heat dissipation (always applies)
        if (mod.heat_level > 0.0f && !mod.overheating) {
            mod.heat_level = std::max(0.0f, mod.heat_level - mod.dissipation_rate * delta_time);
        }

        // Accumulate damage when above threshold
        if (mod.heat_level >= mod.damage_threshold) {
            float excess = mod.heat_level - mod.damage_threshold;
            mod.damage_accumulated += excess * delta_time * 0.1f;
        }

        // Burnout at 100 heat
        if (mod.heat_level >= 100.0f) {
            mod.heat_level = 100.0f;
            mod.burned_out = true;
            mod.overheating = false;
            comp.total_burnouts++;
        }
    }
}

bool ModuleOverheatSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ModuleOverheat>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ModuleOverheatSystem::addModule(const std::string& entity_id,
    const std::string& module_id, float heat_per_cycle,
    float dissipation_rate, float damage_threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    // No duplicate module IDs
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::ModuleOverheat::ModuleSlot slot;
    slot.module_id = module_id;
    slot.heat_per_cycle = heat_per_cycle;
    slot.dissipation_rate = dissipation_rate;
    slot.damage_threshold = damage_threshold;
    comp->modules.push_back(slot);
    return true;
}

bool ModuleOverheatSystem::setOverheating(const std::string& entity_id,
    const std::string& module_id, bool overheat) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.burned_out) return false;
            m.overheating = overheat;
            return true;
        }
    }
    return false;
}

bool ModuleOverheatSystem::cycleModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.burned_out) return false;
            if (m.overheating) {
                m.heat_level += m.heat_per_cycle;
            }
            return true;
        }
    }
    return false;
}

int ModuleOverheatSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

float ModuleOverheatSystem::getHeatLevel(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.heat_level;
    }
    return 0.0f;
}

bool ModuleOverheatSystem::isBurnedOut(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.burned_out;
    }
    return false;
}

bool ModuleOverheatSystem::isOverheating(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.overheating;
    }
    return false;
}

int ModuleOverheatSystem::getTotalBurnouts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_burnouts : 0;
}

float ModuleOverheatSystem::getDamageAccumulated(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.damage_accumulated;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
