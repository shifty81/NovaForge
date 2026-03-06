#ifndef NOVAFORGE_SYSTEMS_ENCOUNTER_SPAWNER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ENCOUNTER_SPAWNER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Spawns dynamic encounters during vertical-slice gameplay phases
 *
 * Creates scripted and procedural encounters tied to the vertical slice
 * flow: pirate ambush during mining, trade escort opportunities, combat
 * challenges, and warp interdiction events. Each encounter has a lifecycle
 * (Pending → Active → Completed/Failed/Expired).
 */
class EncounterSpawnerSystem : public ecs::SingleComponentSystem<components::EncounterState> {
public:
    explicit EncounterSpawnerSystem(ecs::World* world);
    ~EncounterSpawnerSystem() override = default;

    std::string getName() const override { return "EncounterSpawnerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& system_id);

    // Encounter definition
    bool addEncounter(const std::string& entity_id, const std::string& encounter_id,
                      const std::string& encounter_type, int difficulty, float duration);
    int getEncounterCount(const std::string& entity_id) const;
    bool hasEncounter(const std::string& entity_id, const std::string& encounter_id) const;

    // Encounter lifecycle
    bool activateEncounter(const std::string& entity_id, const std::string& encounter_id,
                           float timestamp);
    bool completeEncounter(const std::string& entity_id, const std::string& encounter_id,
                           float timestamp);
    bool failEncounter(const std::string& entity_id, const std::string& encounter_id,
                       float timestamp);
    int getEncounterStatus(const std::string& entity_id, const std::string& encounter_id) const;

    // Queries
    int getActiveEncounterCount(const std::string& entity_id) const;
    int getCompletedEncounterCount(const std::string& entity_id) const;
    int getFailedEncounterCount(const std::string& entity_id) const;

    // Reward tracking
    bool setEncounterReward(const std::string& entity_id, const std::string& encounter_id,
                            double isk_reward, int loot_count);
    double getTotalRewardsEarned(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EncounterState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ENCOUNTER_SPAWNER_SYSTEM_H
