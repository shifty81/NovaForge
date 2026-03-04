#ifndef NOVAFORGE_SYSTEMS_MODULE_CASCADING_FAILURE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MODULE_CASCADING_FAILURE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Per-module damage tracking with cascading failure propagation
 *
 * Tracks individual module HP and dependencies. When a module is destroyed,
 * dependent modules go offline. Power loss cascades to all connected modules.
 */
class ModuleCascadingFailureSystem : public ecs::SingleComponentSystem<components::ModuleCascadingFailure> {
public:
    explicit ModuleCascadingFailureSystem(ecs::World* world);
    ~ModuleCascadingFailureSystem() override = default;

    std::string getName() const override { return "ModuleCascadingFailureSystem"; }

    bool initializeShip(const std::string& entity_id);
    bool addModule(const std::string& entity_id, const std::string& module_id,
                   const std::string& module_type, float max_hp);
    bool addDependency(const std::string& entity_id, const std::string& module_id,
                       const std::string& depends_on_id);
    bool damageModule(const std::string& entity_id, const std::string& module_id, float damage);
    bool repairModule(const std::string& entity_id, const std::string& module_id, float amount);
    bool isModuleOnline(const std::string& entity_id, const std::string& module_id) const;
    bool isModuleDestroyed(const std::string& entity_id, const std::string& module_id) const;
    float getModuleHP(const std::string& entity_id, const std::string& module_id) const;
    int getOnlineModuleCount(const std::string& entity_id) const;
    int getTotalFailures(const std::string& entity_id) const;
    int getCascadeEvents(const std::string& entity_id) const;
    int getModuleCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ModuleCascadingFailure& mcf, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MODULE_CASCADING_FAILURE_SYSTEM_H
