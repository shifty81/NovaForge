#ifndef NOVAFORGE_SYSTEMS_NPC_SCHEDULE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_SCHEDULE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/npc_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief NPC daily activity schedule system
 *
 * Manages NPC activity schedules with time-of-day driven transitions.
 * NPCs follow configurable schedules (mine, haul, trade, patrol, rest)
 * creating visible economic cycles in the game world.
 */
class NPCScheduleSystem : public ecs::SingleComponentSystem<components::NPCSchedule> {
public:
    explicit NPCScheduleSystem(ecs::World* world);
    ~NPCScheduleSystem() override = default;

    std::string getName() const override { return "NPCScheduleSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addEntry(const std::string& entity_id, int activity,
                  float start_hour, float end_hour,
                  const std::string& location, int priority);
    bool removeEntry(const std::string& entity_id, int index);
    bool clearSchedule(const std::string& entity_id);
    bool setCurrentHour(const std::string& entity_id, float hour);
    bool setDayLength(const std::string& entity_id, float length_seconds);
    int getEntryCount(const std::string& entity_id) const;
    int getCurrentActivity(const std::string& entity_id) const;
    float getCurrentHour(const std::string& entity_id) const;
    int getTransitions(const std::string& entity_id) const;
    int getDaysCompleted(const std::string& entity_id) const;
    float getAdherenceScore(const std::string& entity_id) const;
    std::string getCurrentLocation(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NPCSchedule& sched, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_SCHEDULE_SYSTEM_H
