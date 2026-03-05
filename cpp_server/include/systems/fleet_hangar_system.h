#ifndef NOVAFORGE_SYSTEMS_FLEET_HANGAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_HANGAR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet hangar system (Phase 14)
 *
 * Manages fleet-scale hangars with tiered ship storage, locking,
 * repair, power management, and maintenance costs.
 */
class FleetHangarSystem : public ecs::SingleComponentSystem<components::FleetHangar> {
public:
    explicit FleetHangarSystem(ecs::World* world);
    ~FleetHangarSystem() override = default;

    std::string getName() const override { return "FleetHangarSystem"; }

public:
    // Initialization
    bool initializeHangar(const std::string& entity_id, const std::string& owner_id,
                          const std::string& name, int tier);

    // Ship operations
    bool dockShip(const std::string& entity_id, const std::string& ship_id,
                  const std::string& ship_class, float hull_integrity);
    bool undockShip(const std::string& entity_id, const std::string& ship_id);
    bool lockShip(const std::string& entity_id, const std::string& ship_id);
    bool unlockShip(const std::string& entity_id, const std::string& ship_id);
    bool repairShip(const std::string& entity_id, const std::string& ship_id, float amount);

    // Upgrade
    bool upgradeHangar(const std::string& entity_id);

    // Query
    int getShipCount(const std::string& entity_id) const;
    int getMaxSlots(const std::string& entity_id) const;
    int getTier(const std::string& entity_id) const;

    // Power
    bool setPowerEnabled(const std::string& entity_id, bool enabled);

protected:
    void updateComponent(ecs::Entity& entity, components::FleetHangar& hangar, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_HANGAR_SYSTEM_H
