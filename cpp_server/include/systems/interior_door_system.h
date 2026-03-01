#ifndef NOVAFORGE_SYSTEMS_INTERIOR_DOOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INTERIOR_DOOR_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages interior doors — state machine, access control, pressure seals
 *
 * Handles standard doors, airlocks (pressure-aware), and security doors
 * (access-restricted). Doors animate between open/closed states, support
 * locking/unlocking, and airlocks check pressure differential before opening.
 */
class InteriorDoorSystem : public ecs::System {
public:
    explicit InteriorDoorSystem(ecs::World* world);
    ~InteriorDoorSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "InteriorDoorSystem"; }

    /**
     * @brief Create a door entity connecting two rooms
     */
    bool createDoor(const std::string& door_id,
                    const std::string& interior_id,
                    const std::string& room_a_id,
                    const std::string& room_b_id,
                    int door_type = 0);

    /**
     * @brief Request to open a door (checks access & pressure)
     * @param player_access The access level the player has (empty if none)
     * @return true if the door began opening
     */
    bool requestOpen(const std::string& door_id,
                     const std::string& player_access = "");

    /**
     * @brief Request to close a door
     */
    bool requestClose(const std::string& door_id);

    /**
     * @brief Lock a door (prevents opening)
     */
    bool lockDoor(const std::string& door_id);

    /**
     * @brief Unlock a door
     */
    bool unlockDoor(const std::string& door_id);

    /**
     * @brief Set the pressure on side A or B of a door
     */
    bool setPressure(const std::string& door_id, float pressure_a, float pressure_b);

    /**
     * @brief Get the current state of a door
     */
    int getDoorState(const std::string& door_id) const;

    /**
     * @brief Get the open progress of a door (0..1)
     */
    float getOpenProgress(const std::string& door_id) const;

    /**
     * @brief Check if a door has a pressure warning (differential too high)
     */
    bool hasPressureWarning(const std::string& door_id) const;

    /**
     * @brief Check if a door is locked
     */
    bool isLocked(const std::string& door_id) const;

    /**
     * @brief Get the name of a door state
     */
    static std::string stateName(int state);

    /**
     * @brief Get the name of a door type
     */
    static std::string typeName(int type);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INTERIOR_DOOR_SYSTEM_H
