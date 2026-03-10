#include "systems/kill_report_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

KillReportSystem::KillReportSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void KillReportSystem::updateComponent(ecs::Entity& /*entity*/,
                                       components::KillReport& comp,
                                       float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool KillReportSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::KillReport>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Recording
// ---------------------------------------------------------------------------

bool KillReportSystem::recordKill(const std::string& entity_id,
                                   const std::string& killer_id,
                                   const std::string& victim_id,
                                   const std::string& ship_type,
                                   float damage_dealt,
                                   const std::string& system_id,
                                   const std::string& location) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    if (static_cast<int>(comp->kills.size()) >= comp->max_reports) {
        comp->kills.erase(comp->kills.begin());
    }

    components::KillReport::KillEntry entry;
    entry.killer_id     = killer_id;
    entry.victim_id     = victim_id;
    entry.ship_type     = ship_type;
    entry.damage_dealt  = damage_dealt;
    entry.timestamp     = comp->elapsed;
    entry.system_id     = system_id;
    entry.location      = location;
    entry.acknowledged  = false;

    comp->kills.push_back(entry);
    comp->total_kills++;
    comp->total_damage_dealt += damage_dealt;
    comp->pending_kill_reports++;
    return true;
}

bool KillReportSystem::recordLoss(const std::string& entity_id,
                                   const std::string& killer_id,
                                   const std::string& victim_id,
                                   const std::string& ship_type,
                                   float damage_received,
                                   const std::string& system_id,
                                   const std::string& location) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    if (static_cast<int>(comp->losses.size()) >= comp->max_reports) {
        comp->losses.erase(comp->losses.begin());
    }

    components::KillReport::KillEntry entry;
    entry.killer_id     = killer_id;
    entry.victim_id     = victim_id;
    entry.ship_type     = ship_type;
    entry.damage_dealt  = damage_received;
    entry.timestamp     = comp->elapsed;
    entry.system_id     = system_id;
    entry.location      = location;
    entry.acknowledged  = false;

    comp->losses.push_back(entry);
    comp->total_losses++;
    comp->total_damage_received += damage_received;
    comp->pending_loss_reports++;
    return true;
}

// ---------------------------------------------------------------------------
// Acknowledgement
// ---------------------------------------------------------------------------

bool KillReportSystem::acknowledgeKills(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& k : comp->kills) k.acknowledged = true;
    comp->pending_kill_reports = 0;
    return true;
}

bool KillReportSystem::acknowledgeLosses(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& l : comp->losses) l.acknowledged = true;
    comp->pending_loss_reports = 0;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int KillReportSystem::getTotalKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

int KillReportSystem::getTotalLosses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_losses : 0;
}

int KillReportSystem::getPendingKillReports(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pending_kill_reports : 0;
}

int KillReportSystem::getPendingLossReports(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pending_loss_reports : 0;
}

int KillReportSystem::getKillEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->kills.size()) : 0;
}

int KillReportSystem::getLossEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->losses.size()) : 0;
}

float KillReportSystem::getTotalDamageDealt(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_damage_dealt : 0.0f;
}

float KillReportSystem::getTotalDamageReceived(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_damage_received : 0.0f;
}

components::KillReport::KillEntry
KillReportSystem::getMostRecentKill(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->kills.empty()) return {};
    return comp->kills.back();
}

components::KillReport::KillEntry
KillReportSystem::getMostRecentLoss(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->losses.empty()) return {};
    return comp->losses.back();
}

} // namespace systems
} // namespace atlas
