#ifndef NOVAFORGE_SYSTEMS_FUEL_BLOCK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FUEL_BLOCK_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Structure fuel block management system
 *
 * Manages fuel reserves for player-owned structures (citadels, refineries,
 * engineering complexes).  Each fuel reserve has a type, quantity, capacity,
 * and consumption rate.  Per-tick the system drains fuel from every
 * reserve; when any reserve reaches zero the structure goes offline.  A
 * low-fuel warning fires when the remaining fuel is below
 * low_fuel_threshold seconds of consumption.  Refueling adds fuel back,
 * capped at max_quantity.
 */
class FuelBlockSystem
    : public ecs::SingleComponentSystem<components::FuelBlockState> {
public:
    explicit FuelBlockSystem(ecs::World* world);
    ~FuelBlockSystem() override = default;

    std::string getName() const override { return "FuelBlockSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Reserve management ---
    bool addReserve(const std::string& entity_id,
                    const std::string& reserve_id,
                    components::FuelBlockState::FuelType fuel_type,
                    float quantity,
                    float max_quantity,
                    float consumption_rate);
    bool removeReserve(const std::string& entity_id,
                       const std::string& reserve_id);
    bool clearReserves(const std::string& entity_id);

    // --- Operations ---
    bool refuel(const std::string& entity_id,
                const std::string& reserve_id,
                float amount);
    bool setConsumptionRate(const std::string& entity_id,
                            const std::string& reserve_id,
                            float rate);
    bool bringOnline(const std::string& entity_id);

    // --- Configuration ---
    bool setStructureId(const std::string& entity_id,
                        const std::string& structure_id);
    bool setLowFuelThreshold(const std::string& entity_id, float threshold);
    bool setMaxReserves(const std::string& entity_id, int max_reserves);

    // --- Queries ---
    int   getReserveCount(const std::string& entity_id) const;
    bool  hasReserve(const std::string& entity_id,
                     const std::string& reserve_id) const;
    float getFuelQuantity(const std::string& entity_id,
                          const std::string& reserve_id) const;
    float getFuelCapacity(const std::string& entity_id,
                          const std::string& reserve_id) const;
    float getConsumptionRate(const std::string& entity_id,
                             const std::string& reserve_id) const;
    bool  isOnline(const std::string& entity_id) const;
    bool  isLowFuel(const std::string& entity_id) const;
    std::string getStructureId(const std::string& entity_id) const;
    int   getTotalRefuels(const std::string& entity_id) const;
    float getTotalFuelConsumed(const std::string& entity_id) const;
    int   getTotalOfflineEvents(const std::string& entity_id) const;
    float getTimeUntilEmpty(const std::string& entity_id,
                            const std::string& reserve_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FuelBlockState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FUEL_BLOCK_SYSTEM_H
