#ifndef NOVAFORGE_SYSTEMS_NPC_BEHAVIOR_SCHEDULER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_BEHAVIOR_SCHEDULER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief NPC daily schedule with behavior state transitions
 *
 * Manages NPC schedules (patrol, trade, mine, dock, idle, combat,
 * flee) based on time-of-day and threat level.  Threat above a
 * threshold forces combat or flee overrides.  Tracks transition
 * statistics and allows dynamic schedule modification.
 */
class NPCBehaviorSchedulerSystem : public ecs::SingleComponentSystem<components::NPCBehaviorSchedulerState> {
public:
    explicit NPCBehaviorSchedulerSystem(ecs::World* world);
    ~NPCBehaviorSchedulerSystem() override = default;

    std::string getName() const override { return "NPCBehaviorSchedulerSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, const std::string& npc_id,
                    const std::string& faction);

    // Schedule management
    bool addScheduleEntry(const std::string& entity_id, const std::string& label,
                          int behavior, float start_hour, float duration_hours);
    bool removeScheduleEntry(const std::string& entity_id, const std::string& label);
    bool clearSchedule(const std::string& entity_id);

    // State control
    bool setGameHour(const std::string& entity_id, float hour);
    bool setThreatLevel(const std::string& entity_id, float threat);
    bool setThreatThreshold(const std::string& entity_id, float threshold);
    bool forceBehavior(const std::string& entity_id, int behavior);

    // Queries
    int getCurrentBehavior(const std::string& entity_id) const;
    int getPreviousBehavior(const std::string& entity_id) const;
    int getScheduleEntryCount(const std::string& entity_id) const;
    float getGameHour(const std::string& entity_id) const;
    float getThreatLevel(const std::string& entity_id) const;
    int getTotalTransitions(const std::string& entity_id) const;
    int getTotalCombatEntries(const std::string& entity_id) const;
    int getTotalTradeTrips(const std::string& entity_id) const;
    std::string getFaction(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NPCBehaviorSchedulerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_BEHAVIOR_SCHEDULER_SYSTEM_H
