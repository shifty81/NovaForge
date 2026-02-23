#include "systems/fleet_history_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetHistorySystem::FleetHistorySystem(ecs::World* world)
    : System(world) {
}

void FleetHistorySystem::update(float /*delta_time*/) {
    // History system is event-driven via recordEvent
}

void FleetHistorySystem::recordEvent(const std::string& fleet_id,
                                      const std::string& type,
                                      const std::string& desc,
                                      float timestamp,
                                      const std::string& entity_id) {
    auto* entity = world_->getEntity(fleet_id);
    if (!entity) return;
    auto* history = entity->getComponent<components::FleetHistory>();
    if (!history) return;
    history->addEvent(type, desc, timestamp, entity_id);
}

std::vector<components::FleetHistoryEntry>
FleetHistorySystem::getHistory(const std::string& fleet_id, int count) const {
    auto* entity = world_->getEntity(fleet_id);
    if (!entity) return {};
    auto* history = entity->getComponent<components::FleetHistory>();
    if (!history) return {};

    int n = std::min(count, static_cast<int>(history->events.size()));
    return std::vector<components::FleetHistoryEntry>(
        history->events.end() - n, history->events.end());
}

int FleetHistorySystem::getEventCount(const std::string& fleet_id) const {
    auto* entity = world_->getEntity(fleet_id);
    if (!entity) return 0;
    auto* history = entity->getComponent<components::FleetHistory>();
    if (!history) return 0;
    return static_cast<int>(history->events.size());
}

std::vector<components::FleetHistoryEntry>
FleetHistorySystem::getEventsByType(const std::string& fleet_id,
                                     const std::string& type) const {
    auto* entity = world_->getEntity(fleet_id);
    if (!entity) return {};
    auto* history = entity->getComponent<components::FleetHistory>();
    if (!history) return {};

    std::vector<components::FleetHistoryEntry> result;
    for (const auto& event : history->events) {
        if (event.event_type == type) {
            result.push_back(event);
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
