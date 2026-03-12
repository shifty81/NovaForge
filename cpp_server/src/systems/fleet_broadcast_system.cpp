#include "systems/fleet_broadcast_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetBroadcastSystem::FleetBroadcastSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — decrement TTLs, expire broadcasts that reach 0
// ---------------------------------------------------------------------------

void FleetBroadcastSystem::updateComponent(ecs::Entity& /*entity*/,
                                            components::FleetBroadcastState& comp,
                                            float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Decrement TTL and collect expired broadcasts
    auto it = comp.broadcasts.begin();
    while (it != comp.broadcasts.end()) {
        it->ttl -= delta_time;
        if (it->ttl <= 0.0f) {
            comp.total_expired++;
            it = comp.broadcasts.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool FleetBroadcastSystem::initialize(const std::string& entity_id,
                                       const std::string& fleet_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetBroadcastState>();
    comp->fleet_id = fleet_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Broadcast management
// ---------------------------------------------------------------------------

bool FleetBroadcastSystem::sendBroadcast(
        const std::string& entity_id,
        const std::string& broadcast_id,
        components::FleetBroadcastState::BroadcastType type,
        const std::string& sender_id,
        const std::string& target_label,
        float ttl) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (broadcast_id.empty()) return false;
    if (sender_id.empty()) return false;
    if (ttl <= 0.0f) return false;
    if (static_cast<int>(comp->broadcasts.size()) >= comp->max_broadcasts) return false;

    // Duplicate check
    for (const auto& b : comp->broadcasts) {
        if (b.broadcast_id == broadcast_id) return false;
    }

    components::FleetBroadcastState::Broadcast bc;
    bc.broadcast_id = broadcast_id;
    bc.type         = type;
    bc.sender_id    = sender_id;
    bc.target_label = target_label;
    bc.ttl          = ttl;
    comp->broadcasts.push_back(bc);
    comp->total_sent++;
    return true;
}

bool FleetBroadcastSystem::dismissBroadcast(const std::string& entity_id,
                                             const std::string& broadcast_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->broadcasts.begin(), comp->broadcasts.end(),
        [&](const components::FleetBroadcastState::Broadcast& b) {
            return b.broadcast_id == broadcast_id;
        });
    if (it == comp->broadcasts.end()) return false;
    comp->broadcasts.erase(it);
    return true;
}

bool FleetBroadcastSystem::clearBroadcasts(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->broadcasts.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool FleetBroadcastSystem::setFleetId(const std::string& entity_id,
                                       const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int FleetBroadcastSystem::getActiveBroadcastCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->broadcasts.size()) : 0;
}

int FleetBroadcastSystem::getTotalSent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_sent : 0;
}

int FleetBroadcastSystem::getTotalExpired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_expired : 0;
}

std::string FleetBroadcastSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleet_id : "";
}

bool FleetBroadcastSystem::hasBroadcast(const std::string& entity_id,
                                         const std::string& broadcast_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& b : comp->broadcasts) {
        if (b.broadcast_id == broadcast_id) return true;
    }
    return false;
}

int FleetBroadcastSystem::getCountByType(
        const std::string& entity_id,
        components::FleetBroadcastState::BroadcastType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->broadcasts) {
        if (b.type == type) count++;
    }
    return count;
}

float FleetBroadcastSystem::getTtl(const std::string& entity_id,
                                    const std::string& broadcast_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& b : comp->broadcasts) {
        if (b.broadcast_id == broadcast_id) return b.ttl;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
