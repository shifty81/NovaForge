#ifndef NOVAFORGE_SYSTEMS_WING_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WING_MANAGEMENT_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages fleet wings (3 wings × 5 ships)
 *
 * Phase 3: Wing management with wing commander authority,
 * wing roles (Mining/Combat/Logistics/Salvage/Construction),
 * and wing-level morale.
 */
class WingManagementSystem : public ecs::System {
public:
    explicit WingManagementSystem(ecs::World* world);
    ~WingManagementSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WingManagementSystem"; }

    /**
     * @brief Create a new wing in the fleet
     * @param role One of: Mining, Combat, Logistics, Salvage, Construction
     */
    bool createWing(const std::string& fleet_id, const std::string& wing_id, const std::string& role);

    /**
     * @brief Dissolve (remove) a wing from the fleet
     */
    bool dissolveWing(const std::string& fleet_id, const std::string& wing_id);

    /**
     * @brief Assign a member to a wing
     */
    bool assignToWing(const std::string& fleet_id, const std::string& wing_id, const std::string& member_id);

    /**
     * @brief Remove a member from a wing
     */
    bool removeFromWing(const std::string& fleet_id, const std::string& wing_id, const std::string& member_id);

    /**
     * @brief Set the wing commander (must be a current member or auto-added)
     */
    bool setWingCommander(const std::string& fleet_id, const std::string& wing_id, const std::string& commander_id);

    /**
     * @brief Get the role assigned to a wing
     */
    std::string getWingRole(const std::string& fleet_id, const std::string& wing_id) const;

    /**
     * @brief Get all member IDs in a wing
     */
    std::vector<std::string> getWingMembers(const std::string& fleet_id, const std::string& wing_id) const;

    /**
     * @brief Get the commander ID of a wing
     */
    std::string getWingCommander(const std::string& fleet_id, const std::string& wing_id) const;

    /**
     * @brief Get the wing-level morale (0-100)
     */
    float getWingMorale(const std::string& fleet_id, const std::string& wing_id) const;

    /**
     * @brief Get the number of wings in the fleet
     */
    int getWingCount(const std::string& fleet_id) const;

    /// Maximum members allowed per wing
    int max_members_per_wing = 5;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WING_MANAGEMENT_SYSTEM_H
