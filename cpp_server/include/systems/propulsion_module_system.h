#ifndef NOVAFORGE_SYSTEMS_PROPULSION_MODULE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PROPULSION_MODULE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Afterburner / MWD propulsion module system
 *
 * Models EVE Online propulsion module mechanics.  An afterburner (AB)
 * provides a modest speed boost with low cap drain and no signature
 * bloom.  A microwarp drive (MWD) gives a large speed boost but drains
 * significantly more capacitor and inflates signature radius while
 * active.  Modules cycle automatically once activated, consuming
 * capacitor each cycle, and deactivate when capacitor runs out.
 */
class PropulsionModuleSystem
    : public ecs::SingleComponentSystem<components::PropulsionModuleState> {
public:
    explicit PropulsionModuleSystem(ecs::World* world);
    ~PropulsionModuleSystem() override = default;

    std::string getName() const override { return "PropulsionModuleSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    components::PropulsionModuleState::ModuleType type);

    // --- Control ---
    bool activateModule(const std::string& entity_id);
    bool deactivateModule(const std::string& entity_id);

    // --- Configuration ---
    bool setSpeedMultiplier(const std::string& entity_id, float multiplier);
    bool setSignatureBloom(const std::string& entity_id, float bloom);
    bool setCapDrainPerCycle(const std::string& entity_id, float drain);
    bool setCycleTime(const std::string& entity_id, float cycle_time);
    bool setCapacitor(const std::string& entity_id, float capacitor);

    // --- Queries ---
    float getSpeedMultiplier(const std::string& entity_id) const;
    float getSignatureBloom(const std::string& entity_id) const;
    float getEffectiveSpeedMultiplier(const std::string& entity_id) const;
    float getEffectiveSignatureBloom(const std::string& entity_id) const;
    bool  isActive(const std::string& entity_id) const;
    int   getTotalCycles(const std::string& entity_id) const;
    float getActiveDuration(const std::string& entity_id) const;
    float getCapacitorRemaining(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PropulsionModuleState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PROPULSION_MODULE_SYSTEM_H
