#ifndef NOVAFORGE_SYSTEMS_MINING_LASER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MINING_LASER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Mining laser ore extraction with cycle timing and yield
 *
 * Manages mining laser activation, cycle timing, ore extraction
 * per cycle, asteroid depletion, and cumulative yield tracking.
 * Supports strip miners, deep core, and ice harvesting variants.
 */
class MiningLaserSystem : public ecs::SingleComponentSystem<components::MiningLaserState> {
public:
    explicit MiningLaserSystem(ecs::World* world);
    ~MiningLaserSystem() override = default;

    std::string getName() const override { return "MiningLaserSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, const std::string& laser_type,
                    float mining_strength);

    // Operations
    bool startMining(const std::string& entity_id, const std::string& target_asteroid);
    bool stopMining(const std::string& entity_id);
    bool setRange(const std::string& entity_id, float range, float optimal_range);
    bool setCycleDuration(const std::string& entity_id, float duration);
    bool addOreYield(const std::string& entity_id, const std::string& ore_type,
                     float amount);

    // Queries
    bool isMiningActive(const std::string& entity_id) const;
    float getCycleProgress(const std::string& entity_id) const;
    float getTotalOreMined(const std::string& entity_id) const;
    int getTotalCycles(const std::string& entity_id) const;
    int getFailedCycles(const std::string& entity_id) const;
    float getAsteroidRemaining(const std::string& entity_id) const;
    std::string getTargetAsteroid(const std::string& entity_id) const;
    float getMiningStrength(const std::string& entity_id) const;
    int getOreTypeCount(const std::string& entity_id) const;
    float getOreYield(const std::string& entity_id, const std::string& ore_type) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MiningLaserState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MINING_LASER_SYSTEM_H
