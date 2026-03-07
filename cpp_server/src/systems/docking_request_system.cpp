#include "systems/docking_request_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DockingRequestSystem::DockingRequestSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DockingRequestSystem::updateComponent(ecs::Entity& entity,
    components::DockingRequest& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // While granted, advance the tether
    if (comp.phase == components::DockingRequest::Phase::Granted) {
        comp.tether_progress += comp.tether_speed * delta_time;
        if (comp.tether_progress >= 1.0f) {
            comp.tether_progress = 1.0f;
            comp.phase = components::DockingRequest::Phase::Docked;
            comp.total_dockings++;
        }
    }
}

bool DockingRequestSystem::initialize(const std::string& entity_id,
    const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DockingRequest>();
    comp->station_id = station_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool DockingRequestSystem::beginApproach(const std::string& entity_id, float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::DockingRequest::Phase::Idle) return false;
    comp->approach_distance = distance;
    comp->phase = components::DockingRequest::Phase::Approach;
    return true;
}

bool DockingRequestSystem::requestDocking(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::DockingRequest::Phase::Approach) return false;
    if (comp->approach_distance > comp->docking_range) return false;
    comp->phase = components::DockingRequest::Phase::Requested;
    return true;
}

bool DockingRequestSystem::grantDocking(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::DockingRequest::Phase::Requested) return false;
    comp->phase = components::DockingRequest::Phase::Granted;
    comp->tether_progress = 0.0f;
    return true;
}

bool DockingRequestSystem::denyDocking(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::DockingRequest::Phase::Requested) return false;
    comp->phase = components::DockingRequest::Phase::Idle;
    comp->denied_count++;
    return true;
}

bool DockingRequestSystem::undock(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->phase != components::DockingRequest::Phase::Docked) return false;
    comp->phase = components::DockingRequest::Phase::Idle;
    comp->tether_progress = 0.0f;
    comp->approach_distance = 0.0f;
    return true;
}

std::string DockingRequestSystem::getPhase(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "unknown";
    switch (comp->phase) {
        case components::DockingRequest::Phase::Idle:      return "idle";
        case components::DockingRequest::Phase::Approach:   return "approach";
        case components::DockingRequest::Phase::Requested:  return "requested";
        case components::DockingRequest::Phase::Granted:    return "granted";
        case components::DockingRequest::Phase::Docked:     return "docked";
    }
    return "unknown";
}

float DockingRequestSystem::getTetherProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->tether_progress : 0.0f;
}

int DockingRequestSystem::getTotalDockings(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_dockings : 0;
}

int DockingRequestSystem::getDeniedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->denied_count : 0;
}

} // namespace systems
} // namespace atlas
