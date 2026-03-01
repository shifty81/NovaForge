#ifndef NOVAFORGE_SYSTEMS_STATION_HANGAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_HANGAR_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages station hangars — personal, corp, and leased ship storage
 *
 * Hangars provide docking for sub-capital ships (frigate through battleship).
 * Capital+ ships use TetherDockingSystem instead.
 *
 * Features:
 *   - Create personal, corp, or leased hangars at stations
 *   - Store/retrieve ships based on hangar capacity
 *   - Charge daily rental for leased hangars
 *   - Upgrade corp hangars through 4 tiers
 *   - Determine docking method by ship mass (hangar vs tether)
 *   - Provide FPS spawn location for docked players
 */
class StationHangarSystem : public ecs::System {
public:
    explicit StationHangarSystem(ecs::World* world);
    ~StationHangarSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "StationHangarSystem"; }

    /**
     * @brief Create a hangar at a station
     * @param hangar_id   Unique entity id for the hangar
     * @param station_id  Station entity the hangar belongs to
     * @param owner_id    Player or corp that owns/leases
     * @param type        Personal, Corporation, or Leased
     * @return true if hangar was created
     */
    bool createHangar(const std::string& hangar_id,
                      const std::string& station_id,
                      const std::string& owner_id,
                      components::StationHangar::HangarType type =
                          components::StationHangar::HangarType::Leased);

    /**
     * @brief Store a ship in a hangar
     * @return true if ship was stored (hangar has room, ship is small enough)
     */
    bool storeShip(const std::string& hangar_id, const std::string& ship_id);

    /**
     * @brief Retrieve a ship from a hangar
     * @return true if ship was retrieved
     */
    bool retrieveShip(const std::string& hangar_id, const std::string& ship_id);

    /**
     * @brief Upgrade a hangar to the next level
     * @return upgrade cost charged, or 0.0 if upgrade failed
     */
    double upgradeHangar(const std::string& hangar_id);

    /**
     * @brief Determine if a ship should use a hangar (true) or tether arm (false)
     *
     * Capital-class ships (mass >= 50000) use tether arms.
     * All others use hangars.
     */
    bool shouldUseHangar(const std::string& ship_id) const;

    /**
     * @brief Get the FPS spawn position for a player in a hangar
     * @return tuple of (x, y, z) spawn coordinates
     */
    std::tuple<float, float, float> getSpawnPosition(const std::string& hangar_id) const;

    /**
     * @brief Get the number of stored ships in a hangar
     */
    int getStoredShipCount(const std::string& hangar_id) const;

    /**
     * @brief Get the accumulated rental cost for a leased hangar
     */
    double getRentalBalance(const std::string& hangar_id) const;

    /// Mass threshold: ships at or above this mass must use tether docking.
    static constexpr float CAPITAL_MASS_THRESHOLD = 50000.0f;

    /// Upgrade costs per level.
    static constexpr double UPGRADE_COST_STANDARD = 10000.0;
    static constexpr double UPGRADE_COST_ADVANCED = 50000.0;
    static constexpr double UPGRADE_COST_PREMIUM  = 200000.0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_HANGAR_SYSTEM_H
