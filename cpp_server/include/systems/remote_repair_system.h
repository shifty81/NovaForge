#ifndef NOVAFORGE_SYSTEMS_REMOTE_REPAIR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_REMOTE_REPAIR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Remote repair logistics system — shield/armor/hull repair beams.
 *
 * Manages remote repair modules fitted to a logistics ship.  Each module
 * cycles independently; on cycle completion it delivers a repair pulse to
 * a designated target entity.  Aggregate repair totals are tracked per
 * repair layer (shield, armor, hull).  Activating a module requires a
 * non-empty target_id; deactivating clears the target.
 */
class RemoteRepairSystem
    : public ecs::SingleComponentSystem<components::RemoteRepairState> {
public:
    explicit RemoteRepairSystem(ecs::World* world);
    ~RemoteRepairSystem() override = default;

    std::string getName() const override { return "RemoteRepairSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Module management ---
    bool addModule(const std::string& entity_id,
                   const std::string& module_id,
                   components::RemoteRepairState::RepairType type,
                   float rep_amount,
                   float optimal_range,
                   float cycle_time);
    bool removeModule(const std::string& entity_id,
                      const std::string& module_id);

    // --- Activation ---
    bool activateModule(const std::string& entity_id,
                        const std::string& module_id,
                        const std::string& target_id);
    bool deactivateModule(const std::string& entity_id,
                          const std::string& module_id);

    // --- Configuration ---
    bool setRepAmount(const std::string& entity_id,
                      const std::string& module_id,
                      float rep_amount);

    // --- Queries ---
    int   getModuleCount(const std::string& entity_id) const;
    int   getActiveModuleCount(const std::string& entity_id) const;
    bool  isModuleActive(const std::string& entity_id,
                         const std::string& module_id) const;
    float getTotalShieldRepaired(const std::string& entity_id) const;
    float getTotalArmorRepaired(const std::string& entity_id) const;
    float getTotalHullRepaired(const std::string& entity_id) const;
    int   getTotalCycles(const std::string& entity_id) const;
    std::string getTargetId(const std::string& entity_id,
                            const std::string& module_id) const;
    int   getModuleReps(const std::string& entity_id,
                        const std::string& module_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RemoteRepairState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_REMOTE_REPAIR_SYSTEM_H
