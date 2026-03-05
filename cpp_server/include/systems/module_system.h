#ifndef NOVAFORGE_SYSTEMS_MODULE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MODULE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles player module activation, cycling, and capacitor consumption
 *
 * Each tick:
 *  - Advances cycle_progress on active modules
 *  - Consumes capacitor each cycle completion
 *  - Deactivates modules when capacitor is empty
 *  - Applies module effects to ship stats
 */
class ModuleSystem : public ecs::SingleComponentSystem<components::ModuleRack> {
public:
    explicit ModuleSystem(ecs::World* world);
    ~ModuleSystem() override = default;

    std::string getName() const override { return "ModuleSystem"; }

    /**
     * @brief Toggle a module on/off
     * @param slot_type "high", "mid", or "low"
     * @param slot_index Index within the slot type
     * @return true if toggle succeeded
     */
    bool toggleModule(const std::string& entity_id,
                      const std::string& slot_type,
                      int slot_index);

    /**
     * @brief Activate a module (turn on)
     */
    bool activateModule(const std::string& entity_id,
                        const std::string& slot_type,
                        int slot_index);

    /**
     * @brief Deactivate a module (turn off)
     */
    bool deactivateModule(const std::string& entity_id,
                          const std::string& slot_type,
                          int slot_index);

    /**
     * @brief Validate that a ship's fitted modules don't exceed CPU/powergrid
     * @return true if fitting is valid
     */
    bool validateFitting(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::ModuleRack& rack, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MODULE_SYSTEM_H
