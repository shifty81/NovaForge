#include "systems/ship_fitting_validation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {
using SFV = components::ShipFittingValidationState;
}

ShipFittingValidationSystem::ShipFittingValidationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipFittingValidationSystem::updateComponent(ecs::Entity& entity,
    components::ShipFittingValidationState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;

    // Recompute validation errors each tick
    state.validation_errors = 0;
    if (state.cpu_used > state.max_cpu) state.validation_errors++;
    if (state.power_grid_used > state.max_power_grid) state.validation_errors++;
}

bool ShipFittingValidationSystem::initialize(const std::string& entity_id,
    float max_cpu, float max_power_grid, int high_slots, int mid_slots, int low_slots) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipFittingValidationState>();
    comp->max_cpu = max_cpu;
    comp->max_power_grid = max_power_grid;
    comp->high_slots = high_slots;
    comp->mid_slots = mid_slots;
    comp->low_slots = low_slots;
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipFittingValidationSystem::fitModule(const std::string& entity_id,
    const std::string& module_id, const std::string& slot_type,
    float cpu_usage, float power_grid_usage) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Duplicate check
    for (const auto& m : state->fitted_modules) {
        if (m.module_id == module_id) return false;
    }
    // Slot availability check
    int used = 0;
    int max_slot = 0;
    if (slot_type == "high") {
        for (const auto& m : state->fitted_modules) {
            if (m.slot_type == "high") used++;
        }
        max_slot = state->high_slots;
    } else if (slot_type == "mid") {
        for (const auto& m : state->fitted_modules) {
            if (m.slot_type == "mid") used++;
        }
        max_slot = state->mid_slots;
    } else if (slot_type == "low") {
        for (const auto& m : state->fitted_modules) {
            if (m.slot_type == "low") used++;
        }
        max_slot = state->low_slots;
    } else {
        return false; // Unknown slot type
    }
    if (used >= max_slot) return false;

    SFV::FittedModule mod;
    mod.module_id = module_id;
    mod.slot_type = slot_type;
    mod.cpu_usage = cpu_usage;
    mod.power_grid_usage = power_grid_usage;
    state->fitted_modules.push_back(mod);
    state->cpu_used += cpu_usage;
    state->power_grid_used += power_grid_usage;
    return true;
}

bool ShipFittingValidationSystem::unfitModule(const std::string& entity_id,
    const std::string& module_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->fitted_modules.begin(), state->fitted_modules.end(),
        [&](const SFV::FittedModule& m) { return m.module_id == module_id; });
    if (it == state->fitted_modules.end()) return false;
    state->cpu_used -= it->cpu_usage;
    state->power_grid_used -= it->power_grid_usage;
    state->fitted_modules.erase(it);
    return true;
}

bool ShipFittingValidationSystem::hasFittedModule(const std::string& entity_id,
    const std::string& module_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& m : state->fitted_modules) {
        if (m.module_id == module_id) return true;
    }
    return false;
}

int ShipFittingValidationSystem::getFittedModuleCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->fitted_modules.size()) : 0;
}

float ShipFittingValidationSystem::getCpuUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->cpu_used : 0.0f;
}

float ShipFittingValidationSystem::getCpuRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->max_cpu - state->cpu_used) : 0.0f;
}

float ShipFittingValidationSystem::getPowerGridUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->power_grid_used : 0.0f;
}

float ShipFittingValidationSystem::getPowerGridRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->max_power_grid - state->power_grid_used) : 0.0f;
}

int ShipFittingValidationSystem::getHighSlotsUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& m : state->fitted_modules) {
        if (m.slot_type == "high") count++;
    }
    return count;
}

int ShipFittingValidationSystem::getMidSlotsUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& m : state->fitted_modules) {
        if (m.slot_type == "mid") count++;
    }
    return count;
}

int ShipFittingValidationSystem::getLowSlotsUsed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& m : state->fitted_modules) {
        if (m.slot_type == "low") count++;
    }
    return count;
}

int ShipFittingValidationSystem::getHighSlotsRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->high_slots - getHighSlotsUsed(entity_id)) : 0;
}

int ShipFittingValidationSystem::getMidSlotsRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->mid_slots - getMidSlotsUsed(entity_id)) : 0;
}

int ShipFittingValidationSystem::getLowSlotsRemaining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->low_slots - getLowSlotsUsed(entity_id)) : 0;
}

bool ShipFittingValidationSystem::isValidFit(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->cpu_used <= state->max_cpu && state->power_grid_used <= state->max_power_grid;
}

bool ShipFittingValidationSystem::isCpuOverloaded(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->cpu_used > state->max_cpu) : false;
}

bool ShipFittingValidationSystem::isPowerGridOverloaded(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? (state->power_grid_used > state->max_power_grid) : false;
}

int ShipFittingValidationSystem::getValidationErrorCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int errors = 0;
    if (state->cpu_used > state->max_cpu) errors++;
    if (state->power_grid_used > state->max_power_grid) errors++;
    return errors;
}

float ShipFittingValidationSystem::getCpuUtilization(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->max_cpu <= 0.0f) return 0.0f;
    return state->cpu_used / state->max_cpu;
}

float ShipFittingValidationSystem::getPowerGridUtilization(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->max_power_grid <= 0.0f) return 0.0f;
    return state->power_grid_used / state->max_power_grid;
}

} // namespace systems
} // namespace atlas
