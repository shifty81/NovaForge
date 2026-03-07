#ifndef NOVAFORGE_SYSTEMS_FUEL_CONSUMPTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FUEL_CONSUMPTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship fuel management — consumption, refueling, and empty-tank penalties
 *
 * Tracks fuel levels for each ship.  Warp drives, thrusters, and idle
 * systems consume fuel at different rates.  Ships that run dry cannot
 * warp and have reduced thruster output.  Fuel is purchased at stations.
 */
class FuelConsumptionSystem : public ecs::SingleComponentSystem<components::FuelTank> {
public:
    explicit FuelConsumptionSystem(ecs::World* world);
    ~FuelConsumptionSystem() override = default;

    std::string getName() const override { return "FuelConsumptionSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& ship_id,
                    double max_fuel);
    bool setFuelType(const std::string& entity_id, const std::string& fuel_type);
    bool setWarpState(const std::string& entity_id, bool warping);
    bool setThrustState(const std::string& entity_id, bool thrusting);
    bool refuel(const std::string& entity_id, double amount);
    bool setConsumptionRates(const std::string& entity_id, double warp_rate,
                             double thrust_rate, double idle_rate);

    double getCurrentFuel(const std::string& entity_id) const;
    double getMaxFuel(const std::string& entity_id) const;
    double getFuelPercentage(const std::string& entity_id) const;
    std::string getFuelType(const std::string& entity_id) const;
    bool canWarp(const std::string& entity_id) const;
    double getTotalFuelConsumed(const std::string& entity_id) const;
    double getTotalFuelPurchased(const std::string& entity_id) const;
    int getRefuelCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FuelTank& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FUEL_CONSUMPTION_SYSTEM_H
