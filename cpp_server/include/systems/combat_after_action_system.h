#ifndef NOVAFORGE_SYSTEMS_COMBAT_AFTER_ACTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_AFTER_ACTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Post-combat engagement report system with damage statistics and casualty tracking
 *
 * Records per-engagement data: damage dealt/received, time-to-kill, DPS,
 * ISC destroyed, ships lost. Supports engagement lifecycle (start, record hits,
 * finalize) and aggregate queries across all engagements.
 */
class CombatAfterActionSystem : public ecs::SingleComponentSystem<components::CombatAfterActionState> {
public:
    explicit CombatAfterActionSystem(ecs::World* world);
    ~CombatAfterActionSystem() override = default;

    std::string getName() const override { return "CombatAfterActionSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Engagement lifecycle
    bool startEngagement(const std::string& entity_id, const std::string& engagement_id,
                         const std::string& target_name);
    bool recordHit(const std::string& entity_id, const std::string& engagement_id,
                   double damage, bool is_incoming);
    bool finalizeEngagement(const std::string& entity_id, const std::string& engagement_id,
                            float duration, double isc_destroyed);
    int getEngagementCount(const std::string& entity_id) const;

    // Per-engagement queries
    double getEngagementDamageDealt(const std::string& entity_id,
                                    const std::string& engagement_id) const;
    double getEngagementDamageReceived(const std::string& entity_id,
                                       const std::string& engagement_id) const;
    double getEngagementDPS(const std::string& entity_id,
                            const std::string& engagement_id) const;

    // Casualty tracking
    bool recordCasualty(const std::string& entity_id, const std::string& ship_name,
                        double isc_value);
    int getCasualtyCount(const std::string& entity_id) const;
    double getTotalIscLost(const std::string& entity_id) const;

    // Aggregates
    double getTotalDamageDealt(const std::string& entity_id) const;
    double getTotalDamageReceived(const std::string& entity_id) const;
    double getTotalIscDestroyed(const std::string& entity_id) const;
    double getAverageDPS(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CombatAfterActionState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_AFTER_ACTION_SYSTEM_H
