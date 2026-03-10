#ifndef NOVAFORGE_SYSTEMS_KILL_REPORT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_KILL_REPORT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Kill report (kill mail) generation system
 *
 * Records kill and loss events for an entity.  Each entry stores the
 * killer, victim, ship type, damage dealt, and location metadata.
 * Entries are capped at max_reports with oldest-entry eviction.
 * Unacknowledged reports are counted separately so the UI can show a
 * notification badge.
 */
class KillReportSystem
    : public ecs::SingleComponentSystem<components::KillReport> {
public:
    explicit KillReportSystem(ecs::World* world);
    ~KillReportSystem() override = default;

    std::string getName() const override { return "KillReportSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Recording ---
    bool recordKill(const std::string& entity_id,
                    const std::string& killer_id,
                    const std::string& victim_id,
                    const std::string& ship_type,
                    float damage_dealt,
                    const std::string& system_id,
                    const std::string& location);
    bool recordLoss(const std::string& entity_id,
                    const std::string& killer_id,
                    const std::string& victim_id,
                    const std::string& ship_type,
                    float damage_received,
                    const std::string& system_id,
                    const std::string& location);

    // --- Acknowledgement ---
    bool acknowledgeKills(const std::string& entity_id);
    bool acknowledgeLosses(const std::string& entity_id);

    // --- Queries ---
    int  getTotalKills(const std::string& entity_id) const;
    int  getTotalLosses(const std::string& entity_id) const;
    int  getPendingKillReports(const std::string& entity_id) const;
    int  getPendingLossReports(const std::string& entity_id) const;
    int  getKillEntryCount(const std::string& entity_id) const;
    int  getLossEntryCount(const std::string& entity_id) const;
    float getTotalDamageDealt(const std::string& entity_id) const;
    float getTotalDamageReceived(const std::string& entity_id) const;
    components::KillReport::KillEntry
        getMostRecentKill(const std::string& entity_id) const;
    components::KillReport::KillEntry
        getMostRecentLoss(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::KillReport& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_KILL_REPORT_SYSTEM_H
