#ifndef NOVAFORGE_SYSTEMS_INERTIA_MODIFIER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INERTIA_MODIFIER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship inertia modifier system
 *
 * Models EVE Online inertia-modification modules (Nanofiber Internal
 * Structure, Inertial Stabilizers, etc.).  Each module applies an inertia
 * reduction factor subject to EVE-style stacking penalties.  Effective
 * inertia and align time are recomputed whenever the module set changes
 * or a module is toggled on/off.
 *
 * Stacking penalty:  effectiveness_i = base * exp(-(i/2.67)^2)
 * where i is the 0-based index of the module sorted by strength.
 */
class InertiaModifierSystem
    : public ecs::SingleComponentSystem<components::InertiaModifierState> {
public:
    explicit InertiaModifierSystem(ecs::World* world);
    ~InertiaModifierSystem() override = default;

    std::string getName() const override { return "InertiaModifierSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float base_inertia,
                    float base_align_time);

    // --- Module management ---
    bool addModule(const std::string& entity_id,
                   const std::string& module_id,
                   const std::string& name,
                   float inertia_reduction);
    bool removeModule(const std::string& entity_id,
                      const std::string& module_id);
    bool activateModule(const std::string& entity_id,
                        const std::string& module_id);
    bool deactivateModule(const std::string& entity_id,
                          const std::string& module_id);

    // --- Configuration ---
    bool setBaseInertia(const std::string& entity_id, float inertia);
    bool setBaseAlignTime(const std::string& entity_id, float align_time);

    // --- Queries ---
    float getEffectiveInertia(const std::string& entity_id) const;
    float getEffectiveAlignTime(const std::string& entity_id) const;
    float getBaseInertia(const std::string& entity_id) const;
    float getBaseAlignTime(const std::string& entity_id) const;
    int   getModuleCount(const std::string& entity_id) const;
    bool  isModuleActive(const std::string& entity_id,
                         const std::string& module_id) const;
    int   getTotalModifications(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::InertiaModifierState& comp,
                         float delta_time) override;

private:
    void recalculate(components::InertiaModifierState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INERTIA_MODIFIER_SYSTEM_H
