#ifndef NOVAFORGE_SYSTEMS_ASTEROID_MINING_LASER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASTEROID_MINING_LASER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Active mining laser operation — ore extraction and yield
 *
 * Manages fitted mining lasers.  Cycling lasers accumulate ore each
 * tick based on yield, crystal bonus, and remaining hold capacity.
 * Lasers auto-stop when the ore hold is full.
 */
class AsteroidMiningLaserSystem : public ecs::SingleComponentSystem<components::AsteroidMiningLaser> {
public:
    explicit AsteroidMiningLaserSystem(ecs::World* world);
    ~AsteroidMiningLaserSystem() override = default;

    std::string getName() const override { return "AsteroidMiningLaserSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addLaser(const std::string& entity_id, const std::string& laser_id,
                  float yield_per_cycle, float cycle_time);
    bool loadCrystal(const std::string& entity_id, const std::string& laser_id,
                     const std::string& crystal_id, float bonus);
    bool startCycle(const std::string& entity_id, const std::string& laser_id);
    bool stopCycle(const std::string& entity_id, const std::string& laser_id);
    bool setTarget(const std::string& entity_id, const std::string& asteroid_id);

    int getLaserCount(const std::string& entity_id) const;
    bool isCycling(const std::string& entity_id, const std::string& laser_id) const;
    double getOreHoldCurrent(const std::string& entity_id) const;
    double getTotalOreMined(const std::string& entity_id) const;
    int getTotalCyclesCompleted(const std::string& entity_id) const;
    bool isHoldFull(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AsteroidMiningLaser& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASTEROID_MINING_LASER_SYSTEM_H
