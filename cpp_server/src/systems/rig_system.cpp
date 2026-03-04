#include "systems/rig_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

RigSystem::RigSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void RigSystem::updateComponent(ecs::Entity& /*entity*/, components::RigLoadout& loadout, float /*delta_time*/) {
    loadout.total_oxygen = 0.0f;
    loadout.total_power = 0.0f;
    loadout.total_cargo = 0.0f;
    loadout.total_shield = 0.0f;
    loadout.jetpack_fuel = 0.0f;

    for (const auto& mod_id : loadout.installed_module_ids) {
        auto* mod_entity = world_->getEntity(mod_id);
        if (!mod_entity) continue;
        auto* mod = mod_entity->getComponent<components::RigModule>();
        if (!mod) continue;

        switch (mod->type) {
            case components::RigModule::ModuleType::LifeSupport:
                loadout.total_oxygen += 100.0f * static_cast<float>(mod->tier) * mod->efficiency;
                break;
            case components::RigModule::ModuleType::PowerCore:
                loadout.total_power += 50.0f * static_cast<float>(mod->tier) * mod->efficiency;
                break;
            case components::RigModule::ModuleType::CargoPod:
                loadout.total_cargo += 200.0f * static_cast<float>(mod->tier) * mod->efficiency;
                break;
            case components::RigModule::ModuleType::Shield:
                loadout.total_shield += 10.0f * static_cast<float>(mod->tier) * mod->efficiency;
                break;
            case components::RigModule::ModuleType::JetpackTank:
                loadout.jetpack_fuel += 30.0f * static_cast<float>(mod->tier) * mod->efficiency;
                break;
            default:
                break;
        }
    }
}

bool RigSystem::installModule(const std::string& entity_id, const std::string& module_entity_id) {
    auto* loadout = getComponentFor(entity_id);
    if (!loadout) return false;
    if (!loadout->canInstallModule()) return false;
    loadout->installed_module_ids.push_back(module_entity_id);
    return true;
}

bool RigSystem::removeModule(const std::string& entity_id, const std::string& module_entity_id) {
    auto* loadout = getComponentFor(entity_id);
    if (!loadout) return false;

    auto it = std::find(loadout->installed_module_ids.begin(),
                        loadout->installed_module_ids.end(),
                        module_entity_id);
    if (it == loadout->installed_module_ids.end()) return false;
    loadout->installed_module_ids.erase(it);
    return true;
}

int RigSystem::getRackSize(const std::string& entity_id) const {
    const auto* loadout = getComponentFor(entity_id);
    if (!loadout) return 0;
    return loadout->max_slots();
}

int RigSystem::getInstalledCount(const std::string& entity_id) const {
    const auto* loadout = getComponentFor(entity_id);
    if (!loadout) return 0;
    return static_cast<int>(loadout->installed_module_ids.size());
}

} // namespace systems
} // namespace atlas
