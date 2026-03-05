#ifndef NOVAFORGE_SYSTEMS_DYNAMIC_EVENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DYNAMIC_EVENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Random world event management with lifecycle
 *
 * Manages dynamic world events with duration-based lifecycles, intensity
 * scaling, participant tracking, and reward pool accumulation.
 */
class DynamicEventSystem : public ecs::SingleComponentSystem<components::DynamicEvent> {
public:
    explicit DynamicEventSystem(ecs::World* world);
    ~DynamicEventSystem() override = default;

    std::string getName() const override { return "DynamicEventSystem"; }

    bool createEventManager(const std::string& entity_id);
    bool scheduleEvent(const std::string& entity_id, const std::string& event_id,
                       const std::string& type, float duration, float intensity);
    bool startEvent(const std::string& entity_id, const std::string& event_id);
    bool joinEvent(const std::string& entity_id, const std::string& event_id,
                   const std::string& participant);
    bool leaveEvent(const std::string& entity_id, const std::string& event_id,
                    const std::string& participant);
    std::string getEventState(const std::string& entity_id, const std::string& event_id) const;
    std::string getEventType(const std::string& entity_id, const std::string& event_id) const;
    int getParticipantCount(const std::string& entity_id, const std::string& event_id) const;
    float getRewardPool(const std::string& entity_id, const std::string& event_id) const;
    float getIntensity(const std::string& entity_id, const std::string& event_id) const;
    int getActiveEventCount(const std::string& entity_id) const;
    int getTotalCompleted(const std::string& entity_id) const;
    float getElapsedTime(const std::string& entity_id, const std::string& event_id) const;
    bool cancelEvent(const std::string& entity_id, const std::string& event_id);

protected:
    void updateComponent(ecs::Entity& entity, components::DynamicEvent& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DYNAMIC_EVENT_SYSTEM_H
