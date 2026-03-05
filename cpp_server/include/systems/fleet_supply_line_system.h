#ifndef NOVAFORGE_SYSTEMS_FLEET_SUPPLY_LINE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_SUPPLY_LINE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet logistics supply chain management
 *
 * Manages supply depots with fuel and ammo levels that are consumed
 * over time. Supports resupply, consumption, and critical-level detection.
 */
class FleetSupplyLineSystem : public ecs::SingleComponentSystem<components::FleetSupplyLine> {
public:
    explicit FleetSupplyLineSystem(ecs::World* world);
    ~FleetSupplyLineSystem() override = default;

    std::string getName() const override { return "FleetSupplyLineSystem"; }

    bool initializeSupplyLine(const std::string& entity_id);
    bool addDepot(const std::string& entity_id, const std::string& depot_id,
                  const std::string& system_id, float capacity);
    bool removeDepot(const std::string& entity_id, const std::string& depot_id);
    bool resupplyDepot(const std::string& entity_id, const std::string& depot_id,
                       float fuel, float ammo);
    bool consumeSupplies(const std::string& entity_id, const std::string& depot_id,
                         float fuel, float ammo);
    int getDepotCount(const std::string& entity_id) const;
    float getFuelLevel(const std::string& entity_id, const std::string& depot_id) const;
    float getAmmoLevel(const std::string& entity_id, const std::string& depot_id) const;
    int getTotalResupplies(const std::string& entity_id) const;
    bool isDepotCritical(const std::string& entity_id, const std::string& depot_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetSupplyLine& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_SUPPLY_LINE_SYSTEM_H
