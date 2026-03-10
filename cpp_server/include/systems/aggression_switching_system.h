#ifndef NOVAFORGE_SYSTEMS_AGGRESSION_SWITCHING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AGGRESSION_SWITCHING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief NPC aggression target-switching based on threat/DPS
 *
 * Maintains a per-entity threat table and periodically evaluates
 * whether to switch the primary target.  A hysteresis threshold
 * prevents rapid ping-pong between targets.
 */
class AggressionSwitchingSystem : public ecs::SingleComponentSystem<components::AggressionSwitchingState> {
public:
    explicit AggressionSwitchingSystem(ecs::World* world);
    ~AggressionSwitchingSystem() override = default;

    std::string getName() const override { return "AggressionSwitchingSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool addThreatSource(const std::string& entity_id, const std::string& source_id,
                         float dps = 0.0f, float ewar = 0.0f, float proximity = 0.0f);
    bool removeThreatSource(const std::string& entity_id, const std::string& source_id);
    bool applyDamage(const std::string& entity_id, const std::string& source_id, float damage);
    int  getThreatSourceCount(const std::string& entity_id) const;
    float getThreatFor(const std::string& entity_id, const std::string& source_id) const;
    std::string getCurrentTarget(const std::string& entity_id) const;
    int  getTotalSwitches(const std::string& entity_id) const;
    bool setLocked(const std::string& entity_id, bool locked);
    bool isLocked(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AggressionSwitchingState& state,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AGGRESSION_SWITCHING_SYSTEM_H
