#ifndef NOVAFORGE_SYSTEMS_MODULE_POWER_GRID_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MODULE_POWER_GRID_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Enforces runtime CPU and powergrid budgets for fitted modules
 *
 * Validates module online/offline transitions against available CPU/PG.
 * When total capacity drops (e.g. reactor damage), excess modules are
 * forced offline in reverse order.
 */
class ModulePowerGridSystem : public ecs::SingleComponentSystem<components::ModulePowerGrid> {
public:
    explicit ModulePowerGridSystem(ecs::World* world);
    ~ModulePowerGridSystem() override = default;

    std::string getName() const override { return "ModulePowerGridSystem"; }

    bool initializePowerGrid(const std::string& entity_id,
                             float total_cpu = 100.0f, float total_pg = 200.0f);
    bool fitModule(const std::string& entity_id, const std::string& module_id,
                   const std::string& module_name, float cpu, float pg);
    bool setModuleOnline(const std::string& entity_id, const std::string& module_id,
                         bool online);
    bool removeModule(const std::string& entity_id, const std::string& module_id);
    bool setCapacity(const std::string& entity_id, float cpu, float pg);
    float getCpuUsed(const std::string& entity_id) const;
    float getPgUsed(const std::string& entity_id) const;
    float getCpuFree(const std::string& entity_id) const;
    float getPgFree(const std::string& entity_id) const;
    int getModuleCount(const std::string& entity_id) const;
    int getOnlineCount(const std::string& entity_id) const;
    int getModulesForcedOffline(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ModulePowerGrid& mpg,
                         float delta_time) override;

private:
    void recalculateUsage(components::ModulePowerGrid& mpg);
    void enforceCapacity(components::ModulePowerGrid& mpg);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MODULE_POWER_GRID_SYSTEM_H
