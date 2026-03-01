#ifndef NOVAFORGE_SYSTEMS_TETHER_DOCKING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TETHER_DOCKING_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages tether docking arms for capital-class ships
 *
 * Capital ships (Carrier, Dreadnought, Titan) are too large to fit
 * inside a station hangar. Instead they tether to an external docking
 * arm that extends from the station.
 *
 * Tether lifecycle:
 *   1. Ship requests tether → arm begins extending
 *   2. Arm reaches full extension → ship is locked
 *   3. Crew transfer enabled → FPS transition between ship and station
 *   4. Ship requests undock → arm retracts
 *   5. Arm fully retracted → ship is free
 *
 * While tethered the ship receives station shield protection.
 */
class TetherDockingSystem : public ecs::System {
public:
    explicit TetherDockingSystem(ecs::World* world);
    ~TetherDockingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "TetherDockingSystem"; }

    /**
     * @brief Create a tether arm at a station
     * @param arm_id      Unique entity id for the arm
     * @param station_id  Station entity the arm belongs to
     * @param min_ship_mass  Minimum mass to dock here (default: capital threshold)
     * @return true if arm was created
     */
    bool createArm(const std::string& arm_id,
                   const std::string& station_id,
                   float min_ship_mass = 50000.0f);

    /**
     * @brief Begin tethering a capital ship to the arm
     * @return true if tethering started (arm free, ship heavy enough)
     */
    bool beginTether(const std::string& arm_id, const std::string& ship_id);

    /**
     * @brief Begin undocking — starts arm retraction
     * @return true if undock started
     */
    bool beginUndock(const std::string& arm_id);

    /**
     * @brief Check if crew transfer is available (arm fully locked)
     */
    bool isCrewTransferEnabled(const std::string& arm_id) const;

    /**
     * @brief Get the current arm state
     */
    components::TetherDockingArm::ArmState getArmState(const std::string& arm_id) const;

    /**
     * @brief Get the tethered ship entity id
     */
    std::string getTetheredShip(const std::string& arm_id) const;

    /**
     * @brief Check if an arm is occupied
     */
    bool isOccupied(const std::string& arm_id) const;

    /**
     * @brief Get the extend progress (0.0 = retracted, 1.0 = fully extended)
     */
    float getExtendProgress(const std::string& arm_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TETHER_DOCKING_SYSTEM_H
