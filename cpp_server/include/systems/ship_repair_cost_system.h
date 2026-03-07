#ifndef NOVAFORGE_SYSTEMS_SHIP_REPAIR_COST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_REPAIR_COST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship repair cost tracking for the combat→economy loop
 *
 * Accumulates repair costs from damage events (shield/armor/hull).
 * Each damage layer has a different cost rate.  When docked at a
 * station, the player can apply repairs, deducting from their ISC
 * balance.  Supports discount rates (e.g. from standings or station
 * services).
 */
class ShipRepairCostSystem : public ecs::SingleComponentSystem<components::ShipRepairCost> {
public:
    explicit ShipRepairCostSystem(ecs::World* world);
    ~ShipRepairCostSystem() override = default;

    std::string getName() const override { return "ShipRepairCostSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool recordDamage(const std::string& entity_id, const std::string& source_id,
                      float shield_dmg, float armor_dmg, float hull_dmg);
    bool applyRepair(const std::string& entity_id);
    bool setDocked(const std::string& entity_id, bool docked);
    bool setDiscount(const std::string& entity_id, float discount);
    bool setCostRates(const std::string& entity_id, float shield_rate,
                      float armor_rate, float hull_rate);
    double getRepairCost(const std::string& entity_id) const;
    double getTotalIscSpent(const std::string& entity_id) const;
    int getDamageRecordCount(const std::string& entity_id) const;
    int getTotalRepairsCompleted(const std::string& entity_id) const;
    int getTotalDamageEvents(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipRepairCost& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_REPAIR_COST_SYSTEM_H
