#ifndef NOVAFORGE_SYSTEMS_PLAYER_SPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_SPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player spawn point management and respawn lifecycle system
 *
 * Manages spawn location assignment, death recording, and timed respawn
 * countdowns.  While in the Respawning state the system ticks
 * respawn_timer down; when it reaches zero the player is automatically
 * moved back to the Spawned state and spawn_count is incremented.
 */
class PlayerSpawnSystem
    : public ecs::SingleComponentSystem<components::PlayerSpawn> {
public:
    explicit PlayerSpawnSystem(ecs::World* world);
    ~PlayerSpawnSystem() override = default;

    std::string getName() const override { return "PlayerSpawnSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& spawn_location);
    bool spawnPlayer(const std::string& entity_id);
    bool killPlayer(const std::string& entity_id,
                    const std::string& death_location);
    bool beginRespawn(const std::string& entity_id);

    // --- Queries ---
    components::PlayerSpawn::SpawnState
        getState(const std::string& entity_id) const;
    bool isSpawned(const std::string& entity_id) const;
    int  getSpawnCount(const std::string& entity_id) const;
    int  getDeathCount(const std::string& entity_id) const;
    float getRespawnTimer(const std::string& entity_id) const;
    std::string getSpawnLocation(const std::string& entity_id) const;
    std::string getDeathLocation(const std::string& entity_id) const;

    // --- Configuration ---
    bool setSpawnLocation(const std::string& entity_id,
                          const std::string& location);
    bool setRespawnCooldown(const std::string& entity_id,
                            float cooldown);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlayerSpawn& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_SPAWN_SYSTEM_H
