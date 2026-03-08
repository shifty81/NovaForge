#ifndef NOVAFORGE_SYSTEMS_DOCKING_BAY_ALLOCATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DOCKING_BAY_ALLOCATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Station docking bay capacity management and allocation
 *
 * Manages per-station docking bays with size categories (small, medium,
 * large).  Ships request bays and join a priority queue if none are
 * available.  Tracks docking/undocking statistics, average wait times,
 * and queue timeouts.  Essential for the docking loop in the vertical
 * slice.
 */
class DockingBayAllocationSystem : public ecs::SingleComponentSystem<components::DockingBayAllocationState> {
public:
    explicit DockingBayAllocationSystem(ecs::World* world);
    ~DockingBayAllocationSystem() override = default;

    std::string getName() const override { return "DockingBayAllocationSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, const std::string& station_id);

    // Bay management
    bool addBay(const std::string& entity_id, const std::string& bay_id,
                const std::string& bay_size);
    bool removeBay(const std::string& entity_id, const std::string& bay_id);

    // Docking operations
    bool requestDocking(const std::string& entity_id, const std::string& ship_id,
                        const std::string& required_size, int priority);
    bool assignBay(const std::string& entity_id, const std::string& ship_id);
    bool releaseBay(const std::string& entity_id, const std::string& ship_id);

    // Queries
    int getTotalBays(const std::string& entity_id) const;
    int getOccupiedBays(const std::string& entity_id) const;
    int getFreeBays(const std::string& entity_id, const std::string& bay_size) const;
    int getQueueLength(const std::string& entity_id) const;
    bool isShipDocked(const std::string& entity_id, const std::string& ship_id) const;
    int getTotalDockings(const std::string& entity_id) const;
    int getTotalUndockings(const std::string& entity_id) const;
    float getAvgWaitTime(const std::string& entity_id) const;
    std::string getStationId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DockingBayAllocationState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DOCKING_BAY_ALLOCATION_SYSTEM_H
