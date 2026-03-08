#ifndef NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * StarSystemPopulationSystem — manages NPC population density in a star system.
 *
 * Reads/Writes StarSystemPopulationState component.
 *
 * Design:
 *   - On each spawn interval, evaluates whether to spawn a new NPC based on
 *     activity_level and current population vs. max_npcs.
 *   - Distributes spawns across miner/hauler/trader/pirate/security roles
 *     based on a fixed ratio.
 *   - Tracks total spawned and despawned NPCs.
 *   - Activity level modulates spawn frequency (0 = none, 2 = double rate).
 */
class StarSystemPopulationSystem : public ecs::SingleComponentSystem<components::StarSystemPopulationState> {
public:
    explicit StarSystemPopulationSystem(ecs::World* world);
    ~StarSystemPopulationSystem() override = default;

    std::string getName() const override { return "StarSystemPopulationSystem"; }

    /// Try to spawn an NPC of a given role. Returns false if at max capacity.
    bool spawnNPC(const std::string& entity_id, const std::string& role);

    /// Remove an NPC of a given role. Returns false if none active.
    bool despawnNPC(const std::string& entity_id, const std::string& role);

    /// Query helpers
    int getCurrentPopulation(const std::string& entity_id) const;
    int getRoleCount(const std::string& entity_id, const std::string& role) const;
    float getActivityLevel(const std::string& entity_id) const;
    bool isAtCapacity(const std::string& entity_id) const;
    int getTotalSpawned(const std::string& entity_id) const;
    int getTotalDespawned(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StarSystemPopulationState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STAR_SYSTEM_POPULATION_SYSTEM_H
