#ifndef NOVAFORGE_SYSTEMS_MINING_LASER_CYCLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MINING_LASER_CYCLE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Mining laser cycle management with progress tracking and cargo transfer
 *
 * Manages individual mining laser activation cycles on target asteroids.
 * Each laser progresses through a cycle based on cycle_time; on completion
 * the yield is transferred to cargo hold. Respects cargo capacity limits.
 */
class MiningLaserCycleSystem : public ecs::SingleComponentSystem<components::MiningLaserCycle> {
public:
    explicit MiningLaserCycleSystem(ecs::World* world);
    ~MiningLaserCycleSystem() override = default;

    std::string getName() const override { return "MiningLaserCycleSystem"; }

    bool initializeMining(const std::string& entity_id, const std::string& cargo_entity_id,
                          float cargo_capacity);
    bool startLaser(const std::string& entity_id, const std::string& laser_id,
                    const std::string& target_asteroid_id, const std::string& ore_type,
                    float cycle_time, float yield_per_cycle);
    bool stopLaser(const std::string& entity_id, const std::string& laser_id);
    int getActiveLaserCount(const std::string& entity_id) const;
    float getLaserProgress(const std::string& entity_id, const std::string& laser_id) const;
    int getTotalCyclesCompleted(const std::string& entity_id) const;
    float getTotalOreMined(const std::string& entity_id) const;
    float getCargoUsed(const std::string& entity_id) const;
    float getCargoRemaining(const std::string& entity_id) const;
    bool isCargoFull(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MiningLaserCycle& mlc, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MINING_LASER_CYCLE_SYSTEM_H
