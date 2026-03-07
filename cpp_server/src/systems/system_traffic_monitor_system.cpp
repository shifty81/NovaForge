#include "systems/system_traffic_monitor_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using STM = components::SystemTrafficMonitor;

void recountCategories(STM& comp) {
    comp.player_count = 0;
    comp.npc_trader_count = 0;
    comp.npc_miner_count = 0;
    comp.npc_pirate_count = 0;
    comp.npc_security_count = 0;
    for (const auto& e : comp.entries) {
        switch (e.category) {
            case STM::TrafficCategory::PlayerShip:  comp.player_count++; break;
            case STM::TrafficCategory::NPCTrader:   comp.npc_trader_count++; break;
            case STM::TrafficCategory::NPCMiner:    comp.npc_miner_count++; break;
            case STM::TrafficCategory::NPCPirate:   comp.npc_pirate_count++; break;
            case STM::TrafficCategory::NPCSecurity:  comp.npc_security_count++; break;
        }
    }
}

} // anonymous namespace

SystemTrafficMonitorSystem::SystemTrafficMonitorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SystemTrafficMonitorSystem::updateComponent(ecs::Entity& entity,
    components::SystemTrafficMonitor& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Advance time-in-system for all tracked entities
    for (auto& e : comp.entries) {
        e.time_in_system += delta_time;
    }

    // Snapshot timer
    comp.snapshot_timer += delta_time;
    if (comp.snapshot_timer >= comp.snapshot_interval) {
        comp.snapshot_timer -= comp.snapshot_interval;
        comp.total_snapshots++;
    }

    // Update congestion status
    int total = static_cast<int>(comp.entries.size());
    comp.congested = (static_cast<float>(total) >= comp.congestion_threshold);
}

bool SystemTrafficMonitorSystem::initialize(const std::string& entity_id,
    const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SystemTrafficMonitor>();
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SystemTrafficMonitorSystem::registerEntity(const std::string& entity_id,
    const std::string& tracked_id, int category) {
    auto* stm = getComponentFor(entity_id);
    if (!stm) return false;
    if (static_cast<int>(stm->entries.size()) >= stm->max_entries) return false;

    // Duplicate check
    for (const auto& e : stm->entries) {
        if (e.entity_id == tracked_id) return false;
    }

    STM::TrafficEntry te;
    te.entity_id = tracked_id;
    te.category = static_cast<STM::TrafficCategory>(
        std::max(0, std::min(4, category)));
    te.time_in_system = 0.0f;
    stm->entries.push_back(te);
    stm->total_entities_tracked++;

    recountCategories(*stm);
    return true;
}

bool SystemTrafficMonitorSystem::removeEntity(const std::string& entity_id,
    const std::string& tracked_id) {
    auto* stm = getComponentFor(entity_id);
    if (!stm) return false;

    auto it = std::find_if(stm->entries.begin(), stm->entries.end(),
        [&](const STM::TrafficEntry& e) { return e.entity_id == tracked_id; });
    if (it == stm->entries.end()) return false;
    stm->entries.erase(it);

    recountCategories(*stm);
    return true;
}

int SystemTrafficMonitorSystem::getEntityCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? static_cast<int>(stm->entries.size()) : 0;
}

int SystemTrafficMonitorSystem::getPlayerCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->player_count : 0;
}

int SystemTrafficMonitorSystem::getNPCTraderCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->npc_trader_count : 0;
}

int SystemTrafficMonitorSystem::getNPCMinerCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->npc_miner_count : 0;
}

int SystemTrafficMonitorSystem::getNPCPirateCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->npc_pirate_count : 0;
}

int SystemTrafficMonitorSystem::getNPCSecurityCount(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->npc_security_count : 0;
}

bool SystemTrafficMonitorSystem::isCongested(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->congested : false;
}

int SystemTrafficMonitorSystem::getTotalSnapshots(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->total_snapshots : 0;
}

int SystemTrafficMonitorSystem::getTotalEntitiesTracked(const std::string& entity_id) const {
    auto* stm = getComponentFor(entity_id);
    return stm ? stm->total_entities_tracked : 0;
}

} // namespace systems
} // namespace atlas
