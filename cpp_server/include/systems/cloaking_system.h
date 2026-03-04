#ifndef NOVAFORGE_SYSTEMS_CLOAKING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CLOAKING_SYSTEM_H

#include "ecs/state_machine_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship cloaking device management system
 *
 * Manages cloaking state machine: Inactive → Activating → Cloaked → Deactivating → Inactive.
 * Features: activation/deactivation timers, fuel consumption from capacitor,
 * proximity decloak detection, targeting lockout after decloak, CovertOps warp support.
 */
class CloakingSystem : public ecs::StateMachineSystem<components::CloakingState> {
public:
    explicit CloakingSystem(ecs::World* world);
    ~CloakingSystem() override = default;

    std::string getName() const override { return "CloakingSystem"; }

    // Commands
    bool activateCloak(const std::string& entity_id);
    bool deactivateCloak(const std::string& entity_id);
    bool setProximityRange(const std::string& entity_id, float range);
    bool proximityDecloak(const std::string& entity_id, float nearest_distance);

    // Query API
    std::string getPhase(const std::string& entity_id) const;
    bool isCloaked(const std::string& entity_id) const;
    bool isTargetingLocked(const std::string& entity_id) const;
    float getTargetingLockoutRemaining(const std::string& entity_id) const;
    float getFuelRate(const std::string& entity_id) const;
    bool canWarpCloaked(const std::string& entity_id) const;
    int getDecloakCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CloakingState& cloak, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CLOAKING_SYSTEM_H
