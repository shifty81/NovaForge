#include "systems/overheat_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

OverheatManagementSystem::OverheatManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OverheatManagementSystem::updateComponent(ecs::Entity& /*entity*/,
    components::OverheatManagementState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    float heat_sum = 0.0f;
    for (auto& mod : comp.modules) {
        if (mod.burned_out) {
            heat_sum += mod.heat_level;
            continue;
        }

        if (mod.heat_level > 0.0f) {
            mod.heat_level -= comp.heat_dissipation_rate * delta_time;
            if (mod.heat_level < 0.0f) mod.heat_level = 0.0f;
        }

        if (mod.heat_level >= mod.max_heat && !mod.burned_out) {
            mod.burned_out = true;
            comp.total_burnouts++;
        }

        if (mod.heat_level >= mod.damage_threshold && !mod.overheated) {
            mod.overheated = true;
            comp.total_overheats++;
        }

        heat_sum += mod.heat_level;
    }

    if (!comp.modules.empty()) {
        comp.global_heat = heat_sum / static_cast<float>(comp.modules.size());
    } else {
        comp.global_heat = 0.0f;
    }
}

bool OverheatManagementSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::OverheatManagementState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool OverheatManagementSystem::addModule(const std::string& entity_id,
    const std::string& module_id, float heat_generation, float max_heat) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty()) return false;
    if (heat_generation <= 0.0f) return false;
    if (max_heat <= 0.0f) return false;

    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    components::OverheatManagementState::ModuleHeat mod;
    mod.module_id = module_id;
    mod.heat_generation = heat_generation;
    mod.max_heat = max_heat;
    comp->modules.push_back(mod);
    return true;
}

bool OverheatManagementSystem::removeModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&module_id](const components::OverheatManagementState::ModuleHeat& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    return true;
}

bool OverheatManagementSystem::activateModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.burned_out) return false;
            m.heat_level += m.heat_generation;
            if (m.heat_level > m.max_heat) m.heat_level = m.max_heat;
            return true;
        }
    }
    return false;
}

bool OverheatManagementSystem::setDissipationRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->heat_dissipation_rate = rate;
    return true;
}

bool OverheatManagementSystem::resetModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.burned_out) return false;
            m.heat_level = 0.0f;
            m.overheated = false;
            return true;
        }
    }
    return false;
}

int OverheatManagementSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

float OverheatManagementSystem::getModuleHeat(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.heat_level;
    }
    return 0.0f;
}

bool OverheatManagementSystem::isOverheated(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.overheated;
    }
    return false;
}

bool OverheatManagementSystem::isBurnedOut(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.burned_out;
    }
    return false;
}

float OverheatManagementSystem::getGlobalHeat(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->global_heat : 0.0f;
}

float OverheatManagementSystem::getDissipationRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->heat_dissipation_rate : 0.0f;
}

int OverheatManagementSystem::getTotalOverheats(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_overheats : 0;
}

int OverheatManagementSystem::getTotalBurnouts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_burnouts : 0;
}

} // namespace systems
} // namespace atlas
