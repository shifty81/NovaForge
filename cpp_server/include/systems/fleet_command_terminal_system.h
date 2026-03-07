#ifndef NOVAFORGE_SYSTEMS_FLEET_COMMAND_TERMINAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_COMMAND_TERMINAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet Command Terminal — buildable RTS interface for FPS gameplay
 *
 * Manages the lifecycle of Fleet Command Terminals: placement on structures,
 * boot sequence, player login/logout, fleet order issuing with cooldowns,
 * integrity tracking, and order history.
 *
 * In the FPS-first design, this terminal is the ONLY way to access RTS-style
 * fleet control.  Players walk up to a terminal in first-person, interact
 * with it (switching to command mode), issue fleet orders, then exit back
 * to FPS gameplay.
 *
 * Terminals are buildable objects that can be placed on any owned structure
 * (stations, ships, habitats via GridConstructionSystem).  They require
 * power and can be damaged/destroyed.
 */
class FleetCommandTerminalSystem : public ecs::SingleComponentSystem<components::FleetCommandTerminal> {
public:
    explicit FleetCommandTerminalSystem(ecs::World* world);
    ~FleetCommandTerminalSystem() override = default;

    std::string getName() const override { return "FleetCommandTerminalSystem"; }

public:
    // Lifecycle
    bool initialize(const std::string& entity_id, const std::string& owner_id,
                    const std::string& structure_id);
    bool placeTerminal(const std::string& entity_id);
    bool powerOn(const std::string& entity_id);
    bool powerOff(const std::string& entity_id);

    // Player interaction
    bool loginUser(const std::string& entity_id, const std::string& user_id);
    bool logoutUser(const std::string& entity_id);
    bool enterCommandMode(const std::string& entity_id, const std::string& fleet_id);
    bool exitCommandMode(const std::string& entity_id);

    // Fleet orders
    bool issueOrder(const std::string& entity_id, int order, const std::string& target_id);

    // Damage
    bool damageTerminal(const std::string& entity_id, float amount);
    bool repairTerminal(const std::string& entity_id, float amount);

    // Fleet info update (called by other systems to push fleet state)
    bool updateFleetInfo(const std::string& entity_id, int ship_count,
                         float readiness, float morale);

    // Queries
    std::string getState(const std::string& entity_id) const;
    std::string getActiveUser(const std::string& entity_id) const;
    std::string getCurrentOrder(const std::string& entity_id) const;
    float getIntegrity(const std::string& entity_id) const;
    float getBootProgress(const std::string& entity_id) const;
    int getOrderHistoryCount(const std::string& entity_id) const;
    int getTotalOrdersIssued(const std::string& entity_id) const;
    int getTotalSessions(const std::string& entity_id) const;
    int getFleetShipCount(const std::string& entity_id) const;
    float getFleetReadiness(const std::string& entity_id) const;
    float getFleetMorale(const std::string& entity_id) const;
    bool isOperational(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetCommandTerminal& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_COMMAND_TERMINAL_SYSTEM_H
