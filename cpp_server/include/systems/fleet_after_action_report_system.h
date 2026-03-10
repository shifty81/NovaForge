#ifndef NOVAFORGE_SYSTEMS_FLEET_AFTER_ACTION_REPORT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_AFTER_ACTION_REPORT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Post-combat fleet performance summary system
 *
 * After a combat engagement, provides a structured breakdown of each fleet
 * member's contribution: kills, losses, damage dealt/received, and ISK-
 * value loot collected.  The report lifecycle is Idle → Recording →
 * Finalized.  startReport() resets all counters and opens the Recording
 * state.  Individual record* methods accumulate per-member statistics.
 * finalizeReport() seals the report and increments total_reports.
 *
 * getMVP() returns the pilot_id with the highest damage_dealt.  If the
 * fleet has no recorded damage the first registered member is returned.
 * getFleetEfficiency() = total_kills / (total_kills + total_losses)
 * clamped to [0, 1]; returns 0 when both are zero.
 */
class FleetAfterActionReportSystem
    : public ecs::SingleComponentSystem<components::FleetAfterActionReport> {
public:
    explicit FleetAfterActionReportSystem(ecs::World* world);
    ~FleetAfterActionReportSystem() override = default;

    std::string getName() const override {
        return "FleetAfterActionReportSystem";
    }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);
    bool startReport(const std::string& entity_id);
    bool finalizeReport(const std::string& entity_id, float duration);

    // --- Member registration ---
    bool addMember(const std::string& entity_id,
                   const std::string& pilot_id);

    // --- Event recording (requires Recording state) ---
    bool recordKill(const std::string& entity_id,
                    const std::string& pilot_id);
    bool recordLoss(const std::string& entity_id,
                    const std::string& pilot_id);
    bool recordDamageDealt(const std::string& entity_id,
                           const std::string& pilot_id,
                           float amount);
    bool recordDamageReceived(const std::string& entity_id,
                               const std::string& pilot_id,
                               float amount);
    bool recordLootShared(const std::string& entity_id,
                           const std::string& pilot_id,
                           float isk_value);

    // --- Queries ---
    components::FleetAfterActionReport::State
         getState(const std::string& entity_id) const;
    int  getMemberCount(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    int  getTotalLosses(const std::string& entity_id) const;
    float getTotalDamage(const std::string& entity_id) const;
    float getTotalLoot(const std::string& entity_id) const;
    int   getTotalReports(const std::string& entity_id) const;
    std::string getMVP(const std::string& entity_id) const;
    float getFleetEfficiency(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetAfterActionReport& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_AFTER_ACTION_REPORT_SYSTEM_H
