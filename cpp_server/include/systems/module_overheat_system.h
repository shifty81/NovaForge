#ifndef NOVAFORGE_SYSTEMS_MODULE_OVERHEAT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MODULE_OVERHEAT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Module overheating — risk/reward combat mechanic
 *
 * Players may overheat fitted modules for bonus cycle time at the cost
 * of accumulating heat.  Heat dissipates passively.  If heat exceeds the
 * damage threshold, the module accrues damage; at 100 heat it burns out.
 */
class ModuleOverheatSystem : public ecs::SingleComponentSystem<components::ModuleOverheat> {
public:
    explicit ModuleOverheatSystem(ecs::World* world);
    ~ModuleOverheatSystem() override = default;

    std::string getName() const override { return "ModuleOverheatSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   float heat_per_cycle, float dissipation_rate, float damage_threshold);
    bool setOverheating(const std::string& entity_id, const std::string& module_id, bool overheat);
    bool cycleModule(const std::string& entity_id, const std::string& module_id);

    int getModuleCount(const std::string& entity_id) const;
    float getHeatLevel(const std::string& entity_id, const std::string& module_id) const;
    bool isBurnedOut(const std::string& entity_id, const std::string& module_id) const;
    bool isOverheating(const std::string& entity_id, const std::string& module_id) const;
    int getTotalBurnouts(const std::string& entity_id) const;
    float getDamageAccumulated(const std::string& entity_id, const std::string& module_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ModuleOverheat& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MODULE_OVERHEAT_SYSTEM_H
