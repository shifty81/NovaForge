#include "systems/fleet_morale_resolution_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fleet_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetMoraleResolutionSystem::FleetMoraleResolutionSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void FleetMoraleResolutionSystem::updateComponent(ecs::Entity& /*entity*/, components::FleetMoraleResolution& comp, float delta_time) {
    if (!comp.active) return;

    if (comp.crisis_active) {
        comp.crisis_duration += delta_time;
        if (comp.crisis_duration >= comp.max_crisis_duration) {
            comp.departures++;
            comp.crisis_active = false;
            comp.fleet_morale = std::max(0.0f, comp.fleet_morale - 10.0f);
            comp.crisis_duration = 0.0f;
            comp.current_resolution = "None";
        }
    } else {
        comp.fleet_morale = std::min(100.0f, comp.fleet_morale + comp.recovery_rate * delta_time * comp.ideology_alignment);
        if (comp.fleet_morale < comp.fracture_threshold) {
            comp.crisis_active = true;
            comp.fractures_triggered++;
            comp.crisis_duration = 0.0f;
        }
    }
}

bool FleetMoraleResolutionSystem::initializeFleet(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetMoraleResolution>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetMoraleResolutionSystem::triggerCrisis(const std::string& entity_id) {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return false;
    fm->crisis_active = true;
    fm->fractures_triggered++;
    fm->crisis_duration = 0.0f;
    return true;
}

bool FleetMoraleResolutionSystem::resolveWithMethod(const std::string& entity_id, const std::string& method) {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return false;
    if (!fm->crisis_active) return false;

    if (method != "Compromise" && method != "AuthorityOverride" &&
        method != "Vote" && method != "Mediation") return false;

    fm->current_resolution = method;
    fm->crisis_active = false;
    fm->resolutions_applied++;
    fm->crisis_duration = 0.0f;

    // Morale boost depends on method
    float boost = 0.0f;
    if (method == "Compromise") boost = 10.0f;
    else if (method == "AuthorityOverride") boost = 5.0f;
    else if (method == "Vote") boost = 15.0f;
    else if (method == "Mediation") boost = 20.0f;

    fm->fleet_morale = std::min(100.0f, fm->fleet_morale + boost);
    return true;
}

bool FleetMoraleResolutionSystem::adjustMorale(const std::string& entity_id, float amount) {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return false;
    fm->fleet_morale = std::max(0.0f, std::min(100.0f, fm->fleet_morale + amount));
    return true;
}

bool FleetMoraleResolutionSystem::setIdeologyAlignment(const std::string& entity_id, float alignment) {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return false;
    fm->ideology_alignment = std::max(0.0f, std::min(1.0f, alignment));
    return true;
}

float FleetMoraleResolutionSystem::getFleetMorale(const std::string& entity_id) const {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return 0.0f;
    return fm->fleet_morale;
}

float FleetMoraleResolutionSystem::getIdeologyAlignment(const std::string& entity_id) const {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return 0.0f;
    return fm->ideology_alignment;
}

bool FleetMoraleResolutionSystem::isCrisisActive(const std::string& entity_id) const {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return false;
    return fm->crisis_active;
}

int FleetMoraleResolutionSystem::getDepartures(const std::string& entity_id) const {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return 0;
    return fm->departures;
}

int FleetMoraleResolutionSystem::getResolutionCount(const std::string& entity_id) const {
    auto* fm = getComponentFor(entity_id);
    if (!fm) return 0;
    return fm->resolutions_applied;
}

} // namespace systems
} // namespace atlas
