#ifndef NOVAFORGE_SYSTEMS_COMBAT_ENGAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_ENGAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks combat engagement state lifecycle per entity
 *
 * Manages state transitions: Safe → Engaging → InCombat → Disengaging → Safe.
 * Entities in combat cannot warp or dock. The disengage timer starts when
 * all attackers are cleared and counts down to Safe state.
 */
class CombatEngagementSystem : public ecs::SingleComponentSystem<components::CombatEngagement> {
public:
    explicit CombatEngagementSystem(ecs::World* world);
    ~CombatEngagementSystem() override = default;

    std::string getName() const override { return "CombatEngagementSystem"; }

    bool initializeEngagement(const std::string& entity_id);
    bool addAttacker(const std::string& entity_id, const std::string& attacker_id);
    bool removeAttacker(const std::string& entity_id, const std::string& attacker_id);
    bool setTarget(const std::string& entity_id, const std::string& target_id);
    int getAttackerCount(const std::string& entity_id) const;
    int getState(const std::string& entity_id) const;  // 0=Safe,1=Engaging,2=InCombat,3=Disengaging
    bool isInCombat(const std::string& entity_id) const;
    bool isWarpBlocked(const std::string& entity_id) const;
    bool isDockBlocked(const std::string& entity_id) const;
    float getTimeInState(const std::string& entity_id) const;
    int getEngagementCount(const std::string& entity_id) const;
    float getTotalCombatTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CombatEngagement& ce, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_ENGAGEMENT_SYSTEM_H
