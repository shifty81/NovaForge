#include "systems/fleet_warp_coordinator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetWarpCoordinatorSystem::FleetWarpCoordinatorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetWarpCoordinatorSystem::updateComponent(ecs::Entity& /*entity*/,
    components::FleetWarpCoordinatorState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.warp_initiated) return;

    // Advance member alignment
    for (auto& m : comp.members) {
        if (m.warping) continue;
        if (!m.ready && m.align_time > 0.0f) {
            m.align_progress += delta_time / m.align_time;
            if (m.align_progress >= 1.0f) {
                m.align_progress = 1.0f;
                m.ready = true;
            }
        }
    }

    // Check if all members are ready
    bool all_ready = true;
    for (const auto& m : comp.members) {
        if (!m.ready && !m.warping) { all_ready = false; break; }
    }

    // Countdown once all aligned
    if (all_ready && !comp.warp_active) {
        comp.warp_countdown -= delta_time;
        if (comp.warp_countdown <= 0.0f) {
            comp.warp_active = true;
            comp.warp_countdown = 0.0f;
            comp.total_fleet_warps++;
            for (auto& m : comp.members) {
                m.warping = true;
                comp.total_members_warped++;
            }
        }
    }
}

bool FleetWarpCoordinatorSystem::initialize(const std::string& entity_id,
    const std::string& fleet_id, const std::string& commander_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (fleet_id.empty() || commander_id.empty()) return false;

    auto comp = std::make_unique<components::FleetWarpCoordinatorState>();
    comp->fleet_id = fleet_id;
    comp->commander_id = commander_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetWarpCoordinatorSystem::addMember(const std::string& entity_id,
    const std::string& ship_id, float align_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ship_id.empty()) return false;
    if (align_time <= 0.0f) return false;

    // Check for duplicate
    for (const auto& m : comp->members) {
        if (m.ship_id == ship_id) return false;
    }
    if (static_cast<int>(comp->members.size()) >= comp->max_members) return false;

    components::FleetWarpCoordinatorState::FleetMember member;
    member.ship_id = ship_id;
    member.align_time = align_time;
    comp->members.push_back(member);
    return true;
}

bool FleetWarpCoordinatorSystem::removeMember(const std::string& entity_id,
    const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->members.begin(), comp->members.end(),
        [&ship_id](const components::FleetWarpCoordinatorState::FleetMember& m) {
            return m.ship_id == ship_id;
        });
    if (it == comp->members.end()) return false;
    comp->members.erase(it);
    return true;
}

bool FleetWarpCoordinatorSystem::initiateWarp(const std::string& entity_id,
    const std::string& destination) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (destination.empty()) return false;
    if (comp->members.empty()) return false;
    if (comp->warp_initiated) return false;

    comp->destination = destination;
    comp->warp_initiated = true;
    comp->warp_active = false;
    comp->warp_countdown = comp->warp_countdown_duration;

    // Reset alignment
    for (auto& m : comp->members) {
        m.align_progress = 0.0f;
        m.ready = false;
        m.warping = false;
    }
    return true;
}

bool FleetWarpCoordinatorSystem::cancelWarp(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->warp_initiated) return false;

    comp->warp_initiated = false;
    comp->warp_active = false;
    comp->warp_countdown = 0.0f;
    comp->destination.clear();
    for (auto& m : comp->members) {
        m.align_progress = 0.0f;
        m.ready = false;
        m.warping = false;
    }
    return true;
}

int FleetWarpCoordinatorSystem::getMemberCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->members.size()) : 0;
}

int FleetWarpCoordinatorSystem::getReadyCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->members) {
        if (m.ready) count++;
    }
    return count;
}

bool FleetWarpCoordinatorSystem::isAllReady(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->members.empty()) return false;
    for (const auto& m : comp->members) {
        if (!m.ready) return false;
    }
    return true;
}

bool FleetWarpCoordinatorSystem::isWarpActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->warp_active : false;
}

bool FleetWarpCoordinatorSystem::isWarpInitiated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->warp_initiated : false;
}

float FleetWarpCoordinatorSystem::getMemberAlignment(const std::string& entity_id,
    const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->members) {
        if (m.ship_id == ship_id) return m.align_progress;
    }
    return 0.0f;
}

std::string FleetWarpCoordinatorSystem::getDestination(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->destination : "";
}

int FleetWarpCoordinatorSystem::getTotalFleetWarps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fleet_warps : 0;
}

int FleetWarpCoordinatorSystem::getTotalMembersWarped(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_members_warped : 0;
}

float FleetWarpCoordinatorSystem::getWarpCountdown(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->warp_countdown : 0.0f;
}

} // namespace systems
} // namespace atlas
