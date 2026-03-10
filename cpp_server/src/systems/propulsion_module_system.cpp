#include "systems/propulsion_module_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

PropulsionModuleSystem::PropulsionModuleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void PropulsionModuleSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::PropulsionModuleState& comp,
                                              float delta_time) {
    comp.elapsed += delta_time;
    if (!comp.is_active) return;

    comp.active_duration += delta_time;
    comp.cycle_elapsed   += delta_time;

    // Process completed cycles
    while (comp.cycle_elapsed >= comp.cycle_time) {
        comp.cycle_elapsed -= comp.cycle_time;
        comp.capacitor_remaining -= comp.cap_drain_per_cycle;
        comp.total_cycles++;

        if (comp.capacitor_remaining <= 0.0f) {
            comp.capacitor_remaining = 0.0f;
            comp.is_active = false;
            comp.cycle_elapsed = 0.0f;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool PropulsionModuleSystem::initialize(
        const std::string& entity_id,
        components::PropulsionModuleState::ModuleType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PropulsionModuleState>();
    comp->module_type = type;

    if (type == components::PropulsionModuleState::ModuleType::MicrowarpDrive) {
        comp->speed_multiplier = 5.0f;
        comp->signature_bloom  = 5.0f;
        comp->cap_drain_per_cycle = 50.0f;
    } else {
        comp->speed_multiplier = 1.5f;
        comp->signature_bloom  = 1.0f;
        comp->cap_drain_per_cycle = 10.0f;
    }

    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Control
// ---------------------------------------------------------------------------

bool PropulsionModuleSystem::activateModule(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_active) return false;
    if (comp->capacitor_remaining <= 0.0f) return false;
    comp->is_active = true;
    comp->cycle_elapsed = 0.0f;
    return true;
}

bool PropulsionModuleSystem::deactivateModule(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_active) return false;
    comp->is_active = false;
    comp->cycle_elapsed = 0.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool PropulsionModuleSystem::setSpeedMultiplier(const std::string& entity_id,
                                                 float multiplier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (multiplier <= 0.0f) return false;
    comp->speed_multiplier = multiplier;
    return true;
}

bool PropulsionModuleSystem::setSignatureBloom(const std::string& entity_id,
                                                float bloom) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bloom < 1.0f) return false;
    comp->signature_bloom = bloom;
    return true;
}

bool PropulsionModuleSystem::setCapDrainPerCycle(const std::string& entity_id,
                                                  float drain) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (drain <= 0.0f) return false;
    comp->cap_drain_per_cycle = drain;
    return true;
}

bool PropulsionModuleSystem::setCycleTime(const std::string& entity_id,
                                           float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (cycle_time <= 0.0f) return false;
    comp->cycle_time = cycle_time;
    return true;
}

bool PropulsionModuleSystem::setCapacitor(const std::string& entity_id,
                                           float capacitor) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (capacitor < 0.0f) return false;
    comp->capacitor_remaining = capacitor;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float PropulsionModuleSystem::getSpeedMultiplier(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->speed_multiplier : 1.0f;
}

float PropulsionModuleSystem::getSignatureBloom(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->signature_bloom : 1.0f;
}

float PropulsionModuleSystem::getEffectiveSpeedMultiplier(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 1.0f;
    return comp->is_active ? comp->speed_multiplier : 1.0f;
}

float PropulsionModuleSystem::getEffectiveSignatureBloom(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 1.0f;
    return comp->is_active ? comp->signature_bloom : 1.0f;
}

bool PropulsionModuleSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_active : false;
}

int PropulsionModuleSystem::getTotalCycles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles : 0;
}

float PropulsionModuleSystem::getActiveDuration(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_duration : 0.0f;
}

float PropulsionModuleSystem::getCapacitorRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->capacitor_remaining : 0.0f;
}

} // namespace systems
} // namespace atlas
