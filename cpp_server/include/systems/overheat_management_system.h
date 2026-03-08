#ifndef NOVAFORGE_SYSTEMS_OVERHEAT_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OVERHEAT_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Module heat tracking and overheat management system
 *
 * Manages heat levels for ship modules, tracking heat generation,
 * dissipation, overheat thresholds, and burnout states. Calculates
 * global heat as average of all module heat levels.
 */
class OverheatManagementSystem : public ecs::SingleComponentSystem<components::OverheatManagementState> {
public:
    explicit OverheatManagementSystem(ecs::World* world);
    ~OverheatManagementSystem() override = default;

    std::string getName() const override { return "OverheatManagementSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id);

    // Module management
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   float heat_generation, float max_heat);
    bool removeModule(const std::string& entity_id, const std::string& module_id);

    // Operations
    bool activateModule(const std::string& entity_id, const std::string& module_id);
    bool setDissipationRate(const std::string& entity_id, float rate);
    bool resetModule(const std::string& entity_id, const std::string& module_id);

    // Queries
    int getModuleCount(const std::string& entity_id) const;
    float getModuleHeat(const std::string& entity_id, const std::string& module_id) const;
    bool isOverheated(const std::string& entity_id, const std::string& module_id) const;
    bool isBurnedOut(const std::string& entity_id, const std::string& module_id) const;
    float getGlobalHeat(const std::string& entity_id) const;
    float getDissipationRate(const std::string& entity_id) const;
    int getTotalOverheats(const std::string& entity_id) const;
    int getTotalBurnouts(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::OverheatManagementState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OVERHEAT_MANAGEMENT_SYSTEM_H
