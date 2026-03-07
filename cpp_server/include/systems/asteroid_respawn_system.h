#ifndef NOVAFORGE_SYSTEMS_ASTEROID_RESPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASTEROID_RESPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages asteroid belt regeneration after mining
 *
 * Tracks how many asteroids in a belt have been depleted and gradually
 * respawns them over time.  A configurable delay prevents instant
 * regeneration, and the respawn rate can be tuned per belt.
 */
class AsteroidRespawnSystem : public ecs::SingleComponentSystem<components::AsteroidRespawn> {
public:
    explicit AsteroidRespawnSystem(ecs::World* world);
    ~AsteroidRespawnSystem() override = default;

    std::string getName() const override { return "AsteroidRespawnSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& belt_id,
                    const std::string& system_id, int max_asteroids);
    bool deplete(const std::string& entity_id, int count);
    bool setRespawnRate(const std::string& entity_id, float rate);
    bool setRegenerationDelay(const std::string& entity_id, float delay_seconds);

    std::string getState(const std::string& entity_id) const;
    int getTotalAsteroids(const std::string& entity_id) const;
    int getMaxAsteroids(const std::string& entity_id) const;
    int getDepletedCount(const std::string& entity_id) const;
    float getDepletionPct(const std::string& entity_id) const;
    int getTotalRespawned(const std::string& entity_id) const;
    int getTotalDepleted(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::AsteroidRespawn& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASTEROID_RESPAWN_SYSTEM_H
