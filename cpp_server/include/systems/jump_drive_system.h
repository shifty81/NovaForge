#ifndef NOVAFORGE_SYSTEMS_JUMP_DRIVE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JUMP_DRIVE_SYSTEM_H

#include "ecs/state_machine_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Capital ship jump drive mechanics
 *
 * Manages jump drive state machine: Idle → SpoolingUp → Jumping → Cooldown → Idle.
 * Features: spool-up timer, fuel consumption per light-year, jump fatigue accumulation
 * and decay, cynosural field targeting, jump range validation.
 */
class JumpDriveSystem : public ecs::StateMachineSystem<components::JumpDriveState> {
public:
    explicit JumpDriveSystem(ecs::World* world);
    ~JumpDriveSystem() override = default;

    std::string getName() const override { return "JumpDriveSystem"; }

    // Commands
    bool initiateJump(const std::string& entity_id, const std::string& destination, float distance_ly, const std::string& cyno_id = "");
    bool cancelJump(const std::string& entity_id);
    bool refuel(const std::string& entity_id, float amount);
    bool setCynoTarget(const std::string& entity_id, const std::string& cyno_id);

    // Query API
    std::string getPhase(const std::string& entity_id) const;
    float getFuel(const std::string& entity_id) const;
    float getMaxFuel(const std::string& entity_id) const;
    float getFatigue(const std::string& entity_id) const;
    float getMaxRange(const std::string& entity_id) const;
    float getEffectiveRange(const std::string& entity_id) const;  // reduced by fatigue
    bool canJump(const std::string& entity_id, float distance_ly) const;
    int getTotalJumps(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::JumpDriveState& jd, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JUMP_DRIVE_SYSTEM_H
