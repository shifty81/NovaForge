#ifndef NOVAFORGE_SYSTEMS_FLEET_WARP_COORDINATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_WARP_COORDINATOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet warp alignment, readiness, and synchronized warp
 *
 * Coordinates fleet-wide warp: the commander initiates, members
 * align, and once all are ready the fleet warps together.  Tracks
 * per-member alignment progress and fleet warp statistics.
 */
class FleetWarpCoordinatorSystem : public ecs::SingleComponentSystem<components::FleetWarpCoordinatorState> {
public:
    explicit FleetWarpCoordinatorSystem(ecs::World* world);
    ~FleetWarpCoordinatorSystem() override = default;

    std::string getName() const override { return "FleetWarpCoordinatorSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, const std::string& fleet_id,
                    const std::string& commander_id);

    // Member management
    bool addMember(const std::string& entity_id, const std::string& ship_id,
                   float align_time);
    bool removeMember(const std::string& entity_id, const std::string& ship_id);

    // Warp operations
    bool initiateWarp(const std::string& entity_id, const std::string& destination);
    bool cancelWarp(const std::string& entity_id);

    // Queries
    int getMemberCount(const std::string& entity_id) const;
    int getReadyCount(const std::string& entity_id) const;
    bool isAllReady(const std::string& entity_id) const;
    bool isWarpActive(const std::string& entity_id) const;
    bool isWarpInitiated(const std::string& entity_id) const;
    float getMemberAlignment(const std::string& entity_id, const std::string& ship_id) const;
    std::string getDestination(const std::string& entity_id) const;
    int getTotalFleetWarps(const std::string& entity_id) const;
    int getTotalMembersWarped(const std::string& entity_id) const;
    float getWarpCountdown(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetWarpCoordinatorState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_WARP_COORDINATOR_SYSTEM_H
