#ifndef NOVAFORGE_SYSTEMS_MISSION_CONSEQUENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MISSION_CONSEQUENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/mission_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Mission consequence system (Phase 15)
 *
 * Persistent mission consequences that affect the game world.
 * Completing/failing missions changes faction standings, system security,
 * market prices, and spawns.
 */
class MissionConsequenceSystem : public ecs::SingleComponentSystem<components::MissionConsequence> {
public:
    explicit MissionConsequenceSystem(ecs::World* world);
    ~MissionConsequenceSystem() override = default;

    std::string getName() const override { return "MissionConsequenceSystem"; }

    // Initialization
    bool initializeConsequences(const std::string& entity_id, const std::string& system_id);

    // Consequence management
    bool triggerConsequence(const std::string& entity_id, const std::string& mission_id,
                            components::MissionConsequence::ConsequenceType type,
                            float magnitude, float duration,
                            const std::string& target_faction, bool permanent);

    bool expireConsequence(const std::string& entity_id, const std::string& consequence_id);

    // Query
    int getActiveCount(const std::string& entity_id) const;
    float getMagnitude(const std::string& entity_id,
                       components::MissionConsequence::ConsequenceType type) const;
    bool isConsequenceActive(const std::string& entity_id,
                              const std::string& consequence_id) const;
    int getPermanentCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MissionConsequence& mc, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MISSION_CONSEQUENCE_SYSTEM_H
