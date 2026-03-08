#include "systems/capacitor_warfare_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CapacitorWarfareSystem::CapacitorWarfareSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CapacitorWarfareSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CapacitorWarfareState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& mod : comp.modules) {
        if (!mod.active_cycling) continue;
        if (mod.target_id.empty()) continue;

        mod.cycle_progress += delta_time / mod.cycle_time;
        while (mod.cycle_progress >= 1.0f) {
            mod.cycle_progress -= 1.0f;
            comp.total_cycles_completed++;

            float drain = mod.drain_rate * mod.cycle_time;
            comp.total_energy_drained += drain;

            if (mod.module_type == "nosferatu") {
                // Nosferatu: drain target, receive energy
                comp.total_energy_received += drain * (1.0f - comp.drain_resistance);
            }
            // Neutralizer: drain target only (no self-benefit)
        }
    }
}

bool CapacitorWarfareSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::CapacitorWarfareState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CapacitorWarfareSystem::addModule(const std::string& entity_id,
    const std::string& module_id, const std::string& module_type,
    float drain_rate, float optimal_range, float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty()) return false;
    if (module_type != "neutralizer" && module_type != "nosferatu") return false;
    if (drain_rate <= 0.0f) return false;
    if (optimal_range <= 0.0f) return false;
    if (cycle_time <= 0.0f) return false;

    // Check for duplicate
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    components::CapacitorWarfareState::WarfareModule mod;
    mod.module_id = module_id;
    mod.module_type = module_type;
    mod.drain_rate = drain_rate;
    mod.optimal_range = optimal_range;
    mod.cycle_time = cycle_time;
    comp->modules.push_back(mod);
    return true;
}

bool CapacitorWarfareSystem::removeModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&module_id](const components::CapacitorWarfareState::WarfareModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    return true;
}

bool CapacitorWarfareSystem::activateModule(const std::string& entity_id,
    const std::string& module_id, const std::string& target_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_id.empty()) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.active_cycling) return false; // already active
            m.target_id = target_id;
            m.active_cycling = true;
            m.cycle_progress = 0.0f;
            return true;
        }
    }
    return false;
}

bool CapacitorWarfareSystem::deactivateModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (!m.active_cycling) return false;
            m.active_cycling = false;
            m.cycle_progress = 0.0f;
            m.target_id.clear();
            return true;
        }
    }
    return false;
}

bool CapacitorWarfareSystem::setDrainResistance(const std::string& entity_id,
    float resistance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (resistance < 0.0f || resistance > 1.0f) return false;
    comp->drain_resistance = resistance;
    return true;
}

int CapacitorWarfareSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

int CapacitorWarfareSystem::getActiveModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->modules) {
        if (m.active_cycling) count++;
    }
    return count;
}

float CapacitorWarfareSystem::getDrainResistance(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->drain_resistance : 0.0f;
}

float CapacitorWarfareSystem::getTotalEnergyDrained(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_energy_drained : 0.0f;
}

float CapacitorWarfareSystem::getTotalEnergyReceived(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_energy_received : 0.0f;
}

float CapacitorWarfareSystem::getTotalEnergyLost(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_energy_lost : 0.0f;
}

int CapacitorWarfareSystem::getTotalCyclesCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles_completed : 0;
}

int CapacitorWarfareSystem::getTotalTargetsCapped(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_targets_capped : 0;
}

bool CapacitorWarfareSystem::isModuleActive(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.active_cycling;
    }
    return false;
}

float CapacitorWarfareSystem::getModuleCycleProgress(const std::string& entity_id,
    const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.cycle_progress;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
