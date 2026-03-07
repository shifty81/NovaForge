#ifndef NOVAFORGE_SYSTEMS_NPC_TRADER_SCHEDULER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_TRADER_SCHEDULER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Schedules and manages NPC hauler trade routes
 *
 * Maintains trade routes between stations, spawns NPC hauler
 * entities at configured intervals, and tracks route progress.
 * Each route goes through loading, transit, and unloading phases
 * before completing a delivery.
 */
class NpcTraderSchedulerSystem : public ecs::SingleComponentSystem<components::NpcTraderSchedule> {
public:
    explicit NpcTraderSchedulerSystem(ecs::World* world);
    ~NpcTraderSchedulerSystem() override = default;

    std::string getName() const override { return "NpcTraderSchedulerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& scheduler_id);
    bool addRoute(const std::string& entity_id, const std::string& route_id,
                  const std::string& origin, const std::string& destination,
                  const std::string& cargo_type, float cargo_volume, double cargo_value);
    bool setMaxHaulers(const std::string& entity_id, int max_haulers);
    bool setSpawnInterval(const std::string& entity_id, float interval);

    int getActiveHaulers(const std::string& entity_id) const;
    int getRouteCount(const std::string& entity_id) const;
    int getTotalDeliveries(const std::string& entity_id) const;
    double getTotalCargoValueDelivered(const std::string& entity_id) const;
    std::string getRouteState(const std::string& entity_id, const std::string& route_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NpcTraderSchedule& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_TRADER_SCHEDULER_SYSTEM_H
