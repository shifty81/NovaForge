#ifndef NOVAFORGE_SYSTEMS_ASTEROID_DEPLETION_TRACKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASTEROID_DEPLETION_TRACKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks asteroid belt depletion and respawn cycles
 *
 * Monitors ore extraction from asteroid belts, tracks depletion state,
 * and manages respawn timers. Integrates with the mining system to
 * provide dynamic resource availability based on player activity
 * and security status of the system.
 */
class AsteroidDepletionTrackerSystem : public ecs::SingleComponentSystem<components::AsteroidDepletionState> {
public:
    explicit AsteroidDepletionTrackerSystem(ecs::World* world);
    ~AsteroidDepletionTrackerSystem() override = default;

    std::string getName() const override { return "AsteroidDepletionTrackerSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool initialize(const std::string& entity_id, float total_volume);
    bool extractOre(const std::string& entity_id, float amount);
    bool setActiveMiners(const std::string& entity_id, int count);
    bool setSecurityBonus(const std::string& entity_id, float bonus);
    bool setRespawnRate(const std::string& entity_id, float rate);
    bool setRespawnDelay(const std::string& entity_id, float delay);
    float getRemainingOre(const std::string& entity_id) const;
    float getTotalVolume(const std::string& entity_id) const;
    float getDepletionPercent(const std::string& entity_id) const;
    bool isDepleted(const std::string& entity_id) const;
    int getTimesDepleted(const std::string& entity_id) const;
    int getActiveMiners(const std::string& entity_id) const;
    float getSecurityBonus(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AsteroidDepletionState& ads, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASTEROID_DEPLETION_TRACKER_SYSTEM_H
