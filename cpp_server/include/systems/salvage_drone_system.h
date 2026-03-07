#ifndef NOVAFORGE_SYSTEMS_SALVAGE_DRONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SALVAGE_DRONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Salvage drone management — deploy, cycle, and collect from wrecks
 *
 * Drones are added to the bay, then deployed to a target wreck.  Each
 * tick, deployed drones cycle salvage attempts.  On cycle completion
 * the success_chance determines if the salvage succeeds or fails.
 * Drones auto-return when the wreck is fully salvaged.
 */
class SalvageDroneSystem : public ecs::SingleComponentSystem<components::SalvageDroneBay> {
public:
    explicit SalvageDroneSystem(ecs::World* world);
    ~SalvageDroneSystem() override = default;

    std::string getName() const override { return "SalvageDroneSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addDrone(const std::string& entity_id, const std::string& drone_id,
                  float cycle_time, float success_chance);
    bool deployDrone(const std::string& entity_id, const std::string& drone_id,
                     const std::string& wreck_id);
    bool recallDrone(const std::string& entity_id, const std::string& drone_id);
    bool recallAll(const std::string& entity_id);

    int getDroneCount(const std::string& entity_id) const;
    int getDeployedCount(const std::string& entity_id) const;
    int getTotalSalvages(const std::string& entity_id) const;
    int getTotalFailures(const std::string& entity_id) const;
    std::string getDroneState(const std::string& entity_id,
                              const std::string& drone_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SalvageDroneBay& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SALVAGE_DRONE_SYSTEM_H
