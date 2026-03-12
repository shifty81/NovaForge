#include "systems/drone_logistics_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DroneLogisticsSystem::DroneLogisticsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — auto-process one pending transfer per tick in fleet-deploy mode
// ---------------------------------------------------------------------------

void DroneLogisticsSystem::updateComponent(ecs::Entity& /*entity*/,
                                            components::DroneLogisticsState& comp,
                                            float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.fleet_deploy_mode) {
        for (auto& req : comp.requests) {
            if (!req.completed && comp.active_drones < comp.max_drones) {
                req.completed = true;
                comp.total_transfers_completed++;
                comp.total_items_transferred += static_cast<float>(req.amount);
                comp.active_drones++;
                break; // one per tick
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool DroneLogisticsSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DroneLogisticsState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Transfer management
// ---------------------------------------------------------------------------

bool DroneLogisticsSystem::queue_transfer(const std::string& entity_id,
                                           const std::string& request_id,
                                           const std::string& source_port,
                                           const std::string& dest_port,
                                           const std::string& item_type,
                                           int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (request_id.empty()) return false;
    if (amount <= 0) return false;
    if (static_cast<int>(comp->requests.size()) >= comp->max_requests) return false;

    for (const auto& r : comp->requests) {
        if (r.request_id == request_id) return false;
    }

    components::DroneLogisticsState::TransferRequest req;
    req.request_id = request_id;
    req.source_port = source_port;
    req.dest_port   = dest_port;
    req.item_type   = item_type;
    req.amount      = amount;
    req.started_at  = comp->elapsed;
    comp->requests.push_back(req);
    return true;
}

bool DroneLogisticsSystem::complete_transfer(const std::string& entity_id,
                                              const std::string& request_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& r : comp->requests) {
        if (r.request_id == request_id) {
            if (r.completed) return false;
            r.completed = true;
            comp->total_transfers_completed++;
            comp->total_items_transferred += static_cast<float>(r.amount);
            return true;
        }
    }
    return false;
}

bool DroneLogisticsSystem::cancel_transfer(const std::string& entity_id,
                                            const std::string& request_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->requests.begin(), comp->requests.end(),
                           [&](const auto& r) { return r.request_id == request_id; });
    if (it == comp->requests.end()) return false;
    comp->requests.erase(it);
    return true;
}

bool DroneLogisticsSystem::clear_requests(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->requests.clear();
    comp->active_drones = 0;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool DroneLogisticsSystem::set_fleet_deploy_mode(const std::string& entity_id,
                                                  bool enabled) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fleet_deploy_mode = enabled;
    return true;
}

bool DroneLogisticsSystem::set_max_drones(const std::string& entity_id,
                                           int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count < 0) return false;
    comp->max_drones = count;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int DroneLogisticsSystem::get_request_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->requests.size());
}

int DroneLogisticsSystem::get_pending_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->requests) {
        if (!r.completed) count++;
    }
    return count;
}

bool DroneLogisticsSystem::is_fleet_deploy_mode(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->fleet_deploy_mode;
}

int DroneLogisticsSystem::get_total_transfers_completed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_transfers_completed;
}

int DroneLogisticsSystem::get_max_drones(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_drones;
}

int DroneLogisticsSystem::get_active_drones(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->active_drones;
}

} // namespace systems
} // namespace atlas
