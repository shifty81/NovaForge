#ifndef NOVAFORGE_SYSTEMS_SYSTEM_TRAFFIC_MONITOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SYSTEM_TRAFFIC_MONITOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Monitors NPC and player traffic density per star system
 *
 * Tracks entities entering/leaving a star system by category (players,
 * NPC traders, miners, pirates, security).  Periodically takes traffic
 * snapshots.  Detects congestion when entity count exceeds threshold.
 * Provides situational awareness for the vertical slice.
 */
class SystemTrafficMonitorSystem : public ecs::SingleComponentSystem<components::SystemTrafficMonitor> {
public:
    explicit SystemTrafficMonitorSystem(ecs::World* world);
    ~SystemTrafficMonitorSystem() override = default;

    std::string getName() const override { return "SystemTrafficMonitorSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& system_id);
    bool registerEntity(const std::string& entity_id, const std::string& tracked_id,
                        int category);
    bool removeEntity(const std::string& entity_id, const std::string& tracked_id);
    int getEntityCount(const std::string& entity_id) const;
    int getPlayerCount(const std::string& entity_id) const;
    int getNPCTraderCount(const std::string& entity_id) const;
    int getNPCMinerCount(const std::string& entity_id) const;
    int getNPCPirateCount(const std::string& entity_id) const;
    int getNPCSecurityCount(const std::string& entity_id) const;
    bool isCongested(const std::string& entity_id) const;
    int getTotalSnapshots(const std::string& entity_id) const;
    int getTotalEntitiesTracked(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SystemTrafficMonitor& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SYSTEM_TRAFFIC_MONITOR_SYSTEM_H
