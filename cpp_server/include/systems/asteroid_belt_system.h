#ifndef NOVAFORGE_SYSTEMS_ASTEROID_BELT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASTEROID_BELT_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Asteroid belt management with depletion, respawning, and mining yield
 *
 * Manages asteroid belt entities containing multiple asteroids of varying ore types.
 * Tracks depletion when mined, respawns depleted asteroids on a timer, and
 * provides ore type queries and belt statistics.
 */
class AsteroidBeltSystem : public ecs::System {
public:
    explicit AsteroidBeltSystem(ecs::World* world);
    ~AsteroidBeltSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AsteroidBeltSystem"; }

    bool initializeBelt(const std::string& entity_id, const std::string& belt_id,
                        const std::string& system_id);
    bool addAsteroid(const std::string& entity_id, const std::string& asteroid_id,
                     const std::string& ore_type, float quantity, float richness);
    bool removeAsteroid(const std::string& entity_id, const std::string& asteroid_id);
    float mineAsteroid(const std::string& entity_id, const std::string& asteroid_id,
                       float amount);
    int getAsteroidCount(const std::string& entity_id) const;
    int getDepletedCount(const std::string& entity_id) const;
    float getRemainingOre(const std::string& entity_id, const std::string& asteroid_id) const;
    bool isAsteroidDepleted(const std::string& entity_id, const std::string& asteroid_id) const;
    int getTotalMined(const std::string& entity_id) const;
    int getTotalRespawned(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASTEROID_BELT_SYSTEM_H
