#ifndef NOVAFORGE_SYSTEMS_THERMAL_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_THERMAL_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages module heat generation and dissipation
 *
 * Each tick dissipates heat based on the entity's radiator capacity.
 * Modules generate heat when activated.  Above the warning threshold (75%)
 * accuracy degrades; at overheat (100%) modules are forced offline.
 */
class ThermalManagementSystem : public ecs::SingleComponentSystem<components::ThermalState> {
public:
    explicit ThermalManagementSystem(ecs::World* world);
    ~ThermalManagementSystem() override = default;

    std::string getName() const override { return "ThermalManagementSystem"; }

    bool initializeThermal(const std::string& entity_id, float max_heat = 100.0f,
                           float dissipation_rate = 5.0f);
    bool addHeat(const std::string& entity_id, float amount);
    float getHeatFraction(const std::string& entity_id) const;
    float getCurrentHeat(const std::string& entity_id) const;
    bool isOverheated(const std::string& entity_id) const;
    bool isWarning(const std::string& entity_id) const;
    int getTotalOverheatEvents(const std::string& entity_id) const;
    float getTotalHeatGenerated(const std::string& entity_id) const;
    float getTotalHeatDissipated(const std::string& entity_id) const;
    bool setDissipationRate(const std::string& entity_id, float rate);

protected:
    void updateComponent(ecs::Entity& entity, components::ThermalState& ts,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_THERMAL_MANAGEMENT_SYSTEM_H
