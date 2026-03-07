#include "systems/beacon_navigation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using NB = components::NavigationBeacon;

const char* stateToString(NB::BeaconState s) {
    switch (s) {
        case NB::BeaconState::Online:    return "Online";
        case NB::BeaconState::Degraded:  return "Degraded";
        case NB::BeaconState::Offline:   return "Offline";
        case NB::BeaconState::Destroyed: return "Destroyed";
    }
    return "Unknown";
}

const char* typeToString(NB::BeaconType t) {
    switch (t) {
        case NB::BeaconType::Waypoint:  return "Waypoint";
        case NB::BeaconType::FleetWarp: return "FleetWarp";
        case NB::BeaconType::Emergency: return "Emergency";
        case NB::BeaconType::Survey:    return "Survey";
    }
    return "Unknown";
}

NB::BeaconType stringToType(const std::string& s) {
    if (s == "FleetWarp") return NB::BeaconType::FleetWarp;
    if (s == "Emergency") return NB::BeaconType::Emergency;
    if (s == "Survey")    return NB::BeaconType::Survey;
    return NB::BeaconType::Waypoint;
}

void recalcState(NB& comp) {
    if (comp.state == NB::BeaconState::Destroyed) return;
    if (comp.signal_strength <= 0.0f) {
        comp.state = NB::BeaconState::Offline;
        comp.signal_strength = 0.0f;
    } else if (comp.signal_strength < 0.5f) {
        comp.state = NB::BeaconState::Degraded;
    } else {
        comp.state = NB::BeaconState::Online;
    }
}

} // anonymous namespace

BeaconNavigationSystem::BeaconNavigationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void BeaconNavigationSystem::updateComponent(ecs::Entity& entity,
    components::NavigationBeacon& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Signal degrades over time
    if (comp.state != NB::BeaconState::Destroyed &&
        comp.state != NB::BeaconState::Offline) {
        comp.signal_strength -= comp.degradation_rate * delta_time;
        recalcState(comp);
    }

    // Scan range scales with signal strength
    comp.scan_range = 1000.0f * comp.signal_strength;
}

bool BeaconNavigationSystem::initialize(const std::string& entity_id,
    const std::string& beacon_id, const std::string& owner_id,
    const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NavigationBeacon>();
    comp->beacon_id = beacon_id;
    comp->owner_id = owner_id;
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool BeaconNavigationSystem::setPosition(const std::string& entity_id,
    double x, double y, double z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->x = x;
    comp->y = y;
    comp->z = z;
    return true;
}

bool BeaconNavigationSystem::setLabel(const std::string& entity_id,
    const std::string& label) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->label = label;
    return true;
}

bool BeaconNavigationSystem::setType(const std::string& entity_id,
    const std::string& type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->type = stringToType(type);
    return true;
}

bool BeaconNavigationSystem::setPublic(const std::string& entity_id,
    bool is_public) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->is_public = is_public;
    return true;
}

bool BeaconNavigationSystem::recordWarpTo(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state == NB::BeaconState::Offline ||
        comp->state == NB::BeaconState::Destroyed) return false;
    comp->total_warps_to++;
    return true;
}

bool BeaconNavigationSystem::recordScan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_scans++;
    return true;
}

bool BeaconNavigationSystem::repair(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state == NB::BeaconState::Destroyed) return false;
    comp->signal_strength = 1.0f;
    comp->state = NB::BeaconState::Online;
    comp->scan_range = 1000.0f;
    return true;
}

std::string BeaconNavigationSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return stateToString(comp->state);
}

std::string BeaconNavigationSystem::getLabel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->label;
}

std::string BeaconNavigationSystem::getType(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return typeToString(comp->type);
}

float BeaconNavigationSystem::getSignalStrength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->signal_strength : 0.0f;
}

float BeaconNavigationSystem::getScanRange(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->scan_range : 0.0f;
}

bool BeaconNavigationSystem::isPublic(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_public : false;
}

int BeaconNavigationSystem::getTotalWarpsTo(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_warps_to : 0;
}

int BeaconNavigationSystem::getTotalScans(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scans : 0;
}

} // namespace systems
} // namespace atlas
