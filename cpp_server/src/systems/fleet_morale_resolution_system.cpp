#include "systems/fleet_morale_resolution_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fleet_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetMoraleResolutionSystem::FleetMoraleResolutionSystem(ecs::World* world)
    : System(world) {
}

void FleetMoraleResolutionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FleetMoraleResolution>();
    for (auto* entity : entities) {
        auto* fm = entity->getComponent<components::FleetMoraleResolution>();
        if (!fm || !fm->active) continue;

        if (fm->crisis_active) {
            fm->crisis_duration += delta_time;
            if (fm->crisis_duration >= fm->max_crisis_duration) {
                fm->departures++;
                fm->crisis_active = false;
                fm->fleet_morale = std::max(0.0f, fm->fleet_morale - 10.0f);
                fm->crisis_duration = 0.0f;
                fm->current_resolution = "None";
            }
        } else {
            fm->fleet_morale = std::min(100.0f, fm->fleet_morale + fm->recovery_rate * delta_time * fm->ideology_alignment);
            if (fm->fleet_morale < fm->fracture_threshold) {
                fm->crisis_active = true;
                fm->fractures_triggered++;
                fm->crisis_duration = 0.0f;
            }
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return false;
    fm->crisis_active = true;
    fm->fractures_triggered++;
    fm->crisis_duration = 0.0f;
    return true;
}

bool FleetMoraleResolutionSystem::resolveWithMethod(const std::string& entity_id, const std::string& method) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return false;
    fm->fleet_morale = std::max(0.0f, std::min(100.0f, fm->fleet_morale + amount));
    return true;
}

bool FleetMoraleResolutionSystem::setIdeologyAlignment(const std::string& entity_id, float alignment) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return false;
    fm->ideology_alignment = std::max(0.0f, std::min(1.0f, alignment));
    return true;
}

float FleetMoraleResolutionSystem::getFleetMorale(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return 0.0f;
    return fm->fleet_morale;
}

float FleetMoraleResolutionSystem::getIdeologyAlignment(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return 0.0f;
    return fm->ideology_alignment;
}

bool FleetMoraleResolutionSystem::isCrisisActive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return false;
    return fm->crisis_active;
}

int FleetMoraleResolutionSystem::getDepartures(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return 0;
    return fm->departures;
}

int FleetMoraleResolutionSystem::getResolutionCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* fm = entity->getComponent<components::FleetMoraleResolution>();
    if (!fm) return 0;
    return fm->resolutions_applied;
}

} // namespace systems
} // namespace atlas
