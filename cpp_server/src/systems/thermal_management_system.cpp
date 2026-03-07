#include "systems/thermal_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ThermalManagementSystem::ThermalManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool ThermalManagementSystem::initializeThermal(const std::string& entity_id,
                                                 float max_heat,
                                                 float dissipation_rate) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::ThermalState>()) return false;

    auto ts = std::make_unique<components::ThermalState>();
    ts->max_heat = max_heat;
    ts->dissipation_rate = dissipation_rate;
    entity->addComponent(std::move(ts));
    return true;
}

bool ThermalManagementSystem::addHeat(const std::string& entity_id, float amount) {
    auto* ts = getComponentFor(entity_id);
    if (!ts || !ts->active || amount <= 0.0f) return false;
    ts->current_heat = std::min(ts->current_heat + amount, ts->max_heat);
    ts->total_heat_generated += amount;
    if (ts->current_heat >= ts->max_heat * ts->overheat_threshold) {
        ts->modules_overheated++;
        ts->total_overheat_events++;
    }
    return true;
}

float ThermalManagementSystem::getHeatFraction(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    if (!ts || ts->max_heat <= 0.0f) return 0.0f;
    return ts->current_heat / ts->max_heat;
}

float ThermalManagementSystem::getCurrentHeat(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    return ts ? ts->current_heat : 0.0f;
}

bool ThermalManagementSystem::isOverheated(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    if (!ts) return false;
    return ts->current_heat >= ts->max_heat * ts->overheat_threshold;
}

bool ThermalManagementSystem::isWarning(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    if (!ts) return false;
    return ts->current_heat >= ts->max_heat * ts->heat_warning_threshold;
}

int ThermalManagementSystem::getTotalOverheatEvents(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    return ts ? ts->total_overheat_events : 0;
}

float ThermalManagementSystem::getTotalHeatGenerated(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    return ts ? ts->total_heat_generated : 0.0f;
}

float ThermalManagementSystem::getTotalHeatDissipated(const std::string& entity_id) const {
    auto* ts = getComponentFor(entity_id);
    return ts ? ts->total_heat_dissipated : 0.0f;
}

bool ThermalManagementSystem::setDissipationRate(const std::string& entity_id, float rate) {
    auto* ts = getComponentFor(entity_id);
    if (!ts || rate < 0.0f) return false;
    ts->dissipation_rate = rate;
    return true;
}

void ThermalManagementSystem::updateComponent(ecs::Entity& /*entity*/,
                                               components::ThermalState& ts,
                                               float delta_time) {
    if (!ts.active) return;
    ts.elapsed += delta_time;

    // Dissipate heat
    if (ts.current_heat > 0.0f) {
        float dissipated = std::min(ts.dissipation_rate * delta_time, ts.current_heat);
        ts.current_heat -= dissipated;
        ts.total_heat_dissipated += dissipated;
    }

    // Clear overheat counter if below threshold
    if (ts.current_heat < ts.max_heat * ts.overheat_threshold) {
        ts.modules_overheated = 0;
    }
}

} // namespace systems
} // namespace atlas
