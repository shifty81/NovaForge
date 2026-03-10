#ifndef NOVAFORGE_SYSTEMS_COMBAT_TIMER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_TIMER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief EVE-style aggression / weapon / pod-kill combat timer tracking
 *
 * Manages the three combat-state timers that govern safe-logout and docking
 * eligibility.  Each timer is set to its configured duration when the
 * corresponding action occurs, then counts down each tick.
 *
 * Aggression timer — set when the entity attacks another entity; prevents
 *   safe logoff while active (default 300 s).
 * Weapon timer — set when a weapon is activated; prevents docking or jump
 *   gate usage while active (default 60 s).
 * Pod-kill timer — set after the entity destroys a capsule; the longest
 *   timer reflecting the severity of the act (default 900 s).
 *
 * isInCombat() returns true while any timer is positive.
 * canSafelyUndock() returns false while the weapon timer is active.
 */
class CombatTimerSystem
    : public ecs::SingleComponentSystem<components::CombatTimer> {
public:
    explicit CombatTimerSystem(ecs::World* world);
    ~CombatTimerSystem() override = default;

    std::string getName() const override { return "CombatTimerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Timer triggers ---
    bool triggerAggression(const std::string& entity_id);
    bool triggerWeapon(const std::string& entity_id);
    bool triggerPodKill(const std::string& entity_id);

    // --- Configuration ---
    bool setAggressionDuration(const std::string& entity_id, float seconds);
    bool setWeaponDuration(const std::string& entity_id, float seconds);
    bool setPodKillDuration(const std::string& entity_id, float seconds);

    // --- Queries ---
    bool  isInCombat(const std::string& entity_id) const;
    bool  canSafelyUndock(const std::string& entity_id) const;
    bool  hasAggression(const std::string& entity_id) const;
    bool  hasWeaponTimer(const std::string& entity_id) const;
    bool  hasPodKillTimer(const std::string& entity_id) const;
    float getAggressionTimer(const std::string& entity_id) const;
    float getWeaponTimer(const std::string& entity_id) const;
    float getPodKillTimer(const std::string& entity_id) const;
    int   getTotalAggressions(const std::string& entity_id) const;
    int   getTotalPodKills(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CombatTimer& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_TIMER_SYSTEM_H
