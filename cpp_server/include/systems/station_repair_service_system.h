#ifndef NOVAFORGE_SYSTEMS_STATION_REPAIR_SERVICE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_REPAIR_SERVICE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Repairs docked ships at stations over time
 *
 * Bridges the dock → repair loop in the vertical slice.  When a player
 * docks at a station, this system calculates missing HP across shield,
 * armor, and hull layers, then progressively restores them at the
 * station's repair rate.  Repair cost is deducted from the player's
 * wallet proportional to HP restored.
 */
class StationRepairServiceSystem : public ecs::SingleComponentSystem<components::StationRepairService> {
public:
    explicit StationRepairServiceSystem(ecs::World* world);
    ~StationRepairServiceSystem() override = default;

    std::string getName() const override { return "StationRepairServiceSystem"; }

    /**
     * @brief Initialize repair tracking for an entity
     */
    bool initializeRepair(const std::string& entity_id,
                          const std::string& station_id,
                          float repair_rate = 50.0f,
                          float cost_per_hp = 1.5f);

    /**
     * @brief Begin repair on a docked entity
     * @param shield_missing Missing shield HP to restore
     * @param armor_missing Missing armor HP to restore
     * @param hull_missing Missing hull HP to restore
     * @return true if repair started
     */
    bool startRepair(const std::string& entity_id,
                     float shield_missing,
                     float armor_missing,
                     float hull_missing);

    /**
     * @brief Check if entity is currently being repaired
     */
    bool isRepairing(const std::string& entity_id) const;

    /**
     * @brief Check if repair is complete
     */
    bool isRepairComplete(const std::string& entity_id) const;

    /**
     * @brief Get current repair phase
     */
    int getRepairPhase(const std::string& entity_id) const;

    /**
     * @brief Get total cost accrued so far
     */
    float getTotalCost(const std::string& entity_id) const;

    /**
     * @brief Get total HP repaired so far
     */
    float getTotalHPRepaired(const std::string& entity_id) const;

    /**
     * @brief Cancel ongoing repair
     */
    bool cancelRepair(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::StationRepairService& repair,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_REPAIR_SERVICE_SYSTEM_H
