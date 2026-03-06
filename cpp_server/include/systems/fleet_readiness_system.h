#ifndef NOVAFORGE_SYSTEMS_FLEET_READINESS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_READINESS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Pre-combat fleet readiness validation and aggregate capability assessment
 *
 * Tracks fleet members' combat stats (DPS, tank, capacitor), validates fitting
 * and supply readiness, calculates aggregate fleet DPS/tank, and flags members
 * that are not ready for undock. Used for pre-engagement assessment.
 */
class FleetReadinessSystem : public ecs::SingleComponentSystem<components::FleetReadinessState> {
public:
    explicit FleetReadinessSystem(ecs::World* world);
    ~FleetReadinessSystem() override = default;

    std::string getName() const override { return "FleetReadinessSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& fleet_id);

    // Member management
    bool addMember(const std::string& entity_id, const std::string& member_id,
                   const std::string& ship_name, float dps, float ehp, float capacitor);
    bool removeMember(const std::string& entity_id, const std::string& member_id);
    int getMemberCount(const std::string& entity_id) const;

    // Member readiness
    bool setMemberReady(const std::string& entity_id, const std::string& member_id, bool ready);
    bool isMemberReady(const std::string& entity_id, const std::string& member_id) const;
    int getReadyCount(const std::string& entity_id) const;
    int getNotReadyCount(const std::string& entity_id) const;

    // Aggregate stats
    float getFleetDPS(const std::string& entity_id) const;
    float getFleetEHP(const std::string& entity_id) const;
    float getFleetCapacitor(const std::string& entity_id) const;

    // Fleet-wide readiness
    bool isFleetReady(const std::string& entity_id) const;
    float getReadinessPercentage(const std::string& entity_id) const;

    // Supply tracking
    bool setSupplyLevel(const std::string& entity_id, const std::string& supply_type, float level);
    float getSupplyLevel(const std::string& entity_id, const std::string& supply_type) const;
    bool isSupplyAdequate(const std::string& entity_id, float threshold) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetReadinessState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_READINESS_SYSTEM_H
