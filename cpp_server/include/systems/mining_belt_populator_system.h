#ifndef NOVAFORGE_SYSTEMS_MINING_BELT_POPULATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MINING_BELT_POPULATOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Asteroid belt population management system
 *
 * Manages asteroid entities within mining belts — spawning, depletion
 * tracking, ore extraction, and timer-based respawn cycles.  Supports
 * the vertical-slice mining belt gameplay loop.
 */
class MiningBeltPopulatorSystem : public ecs::SingleComponentSystem<components::MiningBeltPopulator> {
public:
    explicit MiningBeltPopulatorSystem(ecs::World* world);
    ~MiningBeltPopulatorSystem() override = default;

    std::string getName() const override { return "MiningBeltPopulatorSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addAsteroid(const std::string& entity_id, const std::string& asteroid_id,
                     const std::string& ore_type, float quantity, float richness);
    bool removeAsteroid(const std::string& entity_id, const std::string& asteroid_id);
    bool extractOre(const std::string& entity_id, const std::string& asteroid_id, float amount);
    int getAsteroidCount(const std::string& entity_id) const;
    int getDepletedCount(const std::string& entity_id) const;
    float getRemainingOre(const std::string& entity_id, const std::string& asteroid_id) const;
    float getTotalOreExtracted(const std::string& entity_id) const;
    int getTotalMined(const std::string& entity_id) const;
    int getTotalRespawned(const std::string& entity_id) const;
    bool isAsteroidDepleted(const std::string& entity_id, const std::string& asteroid_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MiningBeltPopulator& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MINING_BELT_POPULATOR_SYSTEM_H
