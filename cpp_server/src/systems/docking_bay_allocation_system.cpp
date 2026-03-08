#include "systems/docking_bay_allocation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DockingBayAllocationSystem::DockingBayAllocationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DockingBayAllocationSystem::updateComponent(ecs::Entity& /*entity*/,
    components::DockingBayAllocationState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Advance wait times in queue
    for (auto& entry : comp.wait_queue) {
        entry.wait_time += delta_time;
    }
}

bool DockingBayAllocationSystem::initialize(const std::string& entity_id,
    const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (station_id.empty()) return false;

    auto comp = std::make_unique<components::DockingBayAllocationState>();
    comp->station_id = station_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool DockingBayAllocationSystem::addBay(const std::string& entity_id,
    const std::string& bay_id, const std::string& bay_size) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bay_id.empty()) return false;
    if (bay_size != "small" && bay_size != "medium" && bay_size != "large") return false;

    // Check for duplicate
    for (const auto& b : comp->bays) {
        if (b.bay_id == bay_id) return false;
    }

    if (static_cast<int>(comp->bays.size()) >= comp->max_bays) return false;

    components::DockingBayAllocationState::DockingBay bay;
    bay.bay_id = bay_id;
    bay.bay_size = bay_size;
    comp->bays.push_back(bay);
    return true;
}

bool DockingBayAllocationSystem::removeBay(const std::string& entity_id,
    const std::string& bay_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->bays.begin(), comp->bays.end(),
        [&bay_id](const components::DockingBayAllocationState::DockingBay& b) {
            return b.bay_id == bay_id;
        });
    if (it == comp->bays.end()) return false;

    // Cannot remove an occupied bay
    if (it->occupied) return false;

    comp->bays.erase(it);
    return true;
}

bool DockingBayAllocationSystem::requestDocking(const std::string& entity_id,
    const std::string& ship_id, const std::string& required_size, int priority) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ship_id.empty()) return false;
    if (required_size != "small" && required_size != "medium" && required_size != "large")
        return false;
    if (priority < 0) return false;

    // Already in queue?
    for (const auto& q : comp->wait_queue) {
        if (q.ship_id == ship_id) return false;
    }

    // Already docked?
    for (const auto& b : comp->bays) {
        if (b.assigned_ship == ship_id && b.occupied) return false;
    }

    if (static_cast<int>(comp->wait_queue.size()) >= comp->max_queue) return false;

    components::DockingBayAllocationState::QueueEntry entry;
    entry.ship_id = ship_id;
    entry.required_size = required_size;
    entry.priority = priority;
    comp->wait_queue.push_back(entry);
    return true;
}

bool DockingBayAllocationSystem::assignBay(const std::string& entity_id,
    const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Find ship in queue
    auto qit = std::find_if(comp->wait_queue.begin(), comp->wait_queue.end(),
        [&ship_id](const components::DockingBayAllocationState::QueueEntry& q) {
            return q.ship_id == ship_id;
        });
    if (qit == comp->wait_queue.end()) return false;

    std::string required_size = qit->required_size;
    float wait_time = qit->wait_time;

    // Find a free bay of the required size
    for (auto& bay : comp->bays) {
        if (!bay.occupied && bay.bay_size == required_size) {
            bay.occupied = true;
            bay.assigned_ship = ship_id;

            // Update average wait time
            comp->total_dockings++;
            comp->avg_wait_time = comp->avg_wait_time
                + (wait_time - comp->avg_wait_time) / static_cast<float>(comp->total_dockings);

            comp->wait_queue.erase(qit);
            return true;
        }
    }

    return false; // No free bay of required size
}

bool DockingBayAllocationSystem::releaseBay(const std::string& entity_id,
    const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& bay : comp->bays) {
        if (bay.assigned_ship == ship_id && bay.occupied) {
            bay.occupied = false;
            bay.assigned_ship.clear();
            bay.service_timer = 0.0f;
            comp->total_undockings++;
            return true;
        }
    }
    return false;
}

int DockingBayAllocationSystem::getTotalBays(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->bays.size()) : 0;
}

int DockingBayAllocationSystem::getOccupiedBays(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->bays) {
        if (b.occupied) count++;
    }
    return count;
}

int DockingBayAllocationSystem::getFreeBays(const std::string& entity_id,
    const std::string& bay_size) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->bays) {
        if (!b.occupied && b.bay_size == bay_size) count++;
    }
    return count;
}

int DockingBayAllocationSystem::getQueueLength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->wait_queue.size()) : 0;
}

bool DockingBayAllocationSystem::isShipDocked(const std::string& entity_id,
    const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& b : comp->bays) {
        if (b.assigned_ship == ship_id && b.occupied) return true;
    }
    return false;
}

int DockingBayAllocationSystem::getTotalDockings(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_dockings : 0;
}

int DockingBayAllocationSystem::getTotalUndockings(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_undockings : 0;
}

float DockingBayAllocationSystem::getAvgWaitTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->avg_wait_time : 0.0f;
}

std::string DockingBayAllocationSystem::getStationId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->station_id : "";
}

} // namespace systems
} // namespace atlas
