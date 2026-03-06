#ifndef NOVAFORGE_SYSTEMS_FLEET_COORDINATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_COORDINATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet-level tactical coordination system
 *
 * Coordinates fleet-wide tactics including engagement orders, target
 * priority assignments, combat readiness tracking, and formation
 * coherence. Manages the fleet's tactical state during combat.
 */
class FleetCoordinationSystem : public ecs::SingleComponentSystem<components::FleetCoordination> {
public:
    explicit FleetCoordinationSystem(ecs::World* world);
    ~FleetCoordinationSystem() override = default;

    std::string getName() const override { return "FleetCoordinationSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& fleet_id);
    bool issueOrder(const std::string& entity_id, int order);
    bool assignTarget(const std::string& entity_id, const std::string& target_id, int priority);
    bool removeTarget(const std::string& entity_id, const std::string& target_id);
    bool addShip(const std::string& entity_id, const std::string& ship_id);
    bool removeShip(const std::string& entity_id, const std::string& ship_id);
    bool enterCombat(const std::string& entity_id);
    bool leaveCombat(const std::string& entity_id);
    bool setFormationCoherence(const std::string& entity_id, float value);
    bool setMoraleFactor(const std::string& entity_id, float value);
    int getOrder(const std::string& entity_id) const;
    int getTargetCount(const std::string& entity_id) const;
    int getShipCount(const std::string& entity_id) const;
    float getCombatReadiness(const std::string& entity_id) const;
    std::string getHighestPriorityTarget(const std::string& entity_id) const;
    bool isInCombat(const std::string& entity_id) const;
    int getTotalOrdersIssued(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetCoordination& fc, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_COORDINATION_SYSTEM_H
