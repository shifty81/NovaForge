#ifndef NOVAFORGE_SYSTEMS_SHIP_MAINTENANCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_MAINTENANCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship wear, degradation, and repair scheduling
 *
 * Ships accumulate wear from normal operation and combat.  When
 * hull integrity drops below threshold levels the condition degrades,
 * applying increasing performance penalties.  Docked ships can queue
 * repair orders that tick down in real time.
 */
class ShipMaintenanceSystem : public ecs::SingleComponentSystem<components::ShipMaintenance> {
public:
    explicit ShipMaintenanceSystem(ecs::World* world);
    ~ShipMaintenanceSystem() override = default;

    std::string getName() const override { return "ShipMaintenanceSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& ship_id);
    bool setWearRate(const std::string& entity_id, float wear_rate, float combat_wear_rate);
    bool setCombatState(const std::string& entity_id, bool in_combat);
    bool setDockedState(const std::string& entity_id, bool docked);
    bool queueRepair(const std::string& entity_id, const std::string& module_name,
                     double cost, float time_required);
    bool applyDamage(const std::string& entity_id, float damage_amount);

    std::string getCondition(const std::string& entity_id) const;
    float getHullIntegrity(const std::string& entity_id) const;
    float getPerformancePenalty(const std::string& entity_id) const;
    int getRepairQueueSize(const std::string& entity_id) const;
    int getTotalRepairsCompleted(const std::string& entity_id) const;
    double getTotalRepairCost(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipMaintenance& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_MAINTENANCE_SYSTEM_H
