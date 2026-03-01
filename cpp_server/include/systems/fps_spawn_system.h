#ifndef NOVAFORGE_SYSTEMS_FPS_SPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_SPAWN_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <tuple>

namespace atlas {
namespace systems {

/**
 * @brief Manages FPS spawn points for first-person mode testing
 *
 * When a player transitions to Interior/FPS mode (e.g. after docking),
 * this system determines the spawn location based on context:
 *
 *   - Hangar spawn:  inside the hangar, next to the docked ship
 *   - Station lobby: main concourse of the station
 *   - Ship bridge:   cockpit / bridge area of a ship
 *   - Tether airlock: docking arm airlock for capital ships
 *   - EVA hatch:     exit point for EVA mode
 *
 * Integrates with ViewModeTransitionSystem for FPS mode entry.
 */
class FPSSpawnSystem : public ecs::System {
public:
    explicit FPSSpawnSystem(ecs::World* world);
    ~FPSSpawnSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FPSSpawnSystem"; }

    /**
     * @brief Create a spawn point entity
     * @param spawn_id       Unique entity id
     * @param parent_id      Parent entity (station, ship, or arm)
     * @param context        Where the spawn is (Hangar, StationLobby, etc.)
     * @param x, y, z        Position offset relative to parent
     * @param yaw            Facing direction in degrees
     * @return true if created
     */
    bool createSpawnPoint(const std::string& spawn_id,
                          const std::string& parent_id,
                          components::FPSSpawnPoint::SpawnContext context,
                          float x, float y, float z,
                          float yaw = 0.0f);

    /**
     * @brief Find the best spawn point for a player based on their dock state
     *
     * Logic:
     *   1. If docked at a station with a personal hangar → Hangar spawn
     *   2. If docked via tether arm → TetherAirlock spawn
     *   3. If docked at station but no hangar → StationLobby spawn
     *   4. If on a ship → ShipBridge spawn
     *   5. Fallback → first active spawn point
     *
     * @param player_id  The player entity
     * @return spawn entity id, or empty string if no spawn found
     */
    std::string findSpawnForPlayer(const std::string& player_id) const;

    /**
     * @brief Get the spawn position and yaw for a spawn point
     * @return tuple of (x, y, z, yaw)
     */
    std::tuple<float, float, float, float> getSpawnTransform(
        const std::string& spawn_id) const;

    /**
     * @brief Get the spawn context type
     */
    components::FPSSpawnPoint::SpawnContext getSpawnContext(
        const std::string& spawn_id) const;

    /**
     * @brief Activate or deactivate a spawn point
     */
    bool setSpawnActive(const std::string& spawn_id, bool active);

    /**
     * @brief Human-readable spawn context name
     */
    static std::string contextName(components::FPSSpawnPoint::SpawnContext ctx);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_SPAWN_SYSTEM_H
