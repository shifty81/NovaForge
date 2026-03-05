#ifndef NOVAFORGE_SYSTEMS_VISUAL_FEEDBACK_QUEUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VISUAL_FEEDBACK_QUEUE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Server-side visual feedback effect management
 *
 * Queues visual effects (damage numbers, shield ripples, heal indicators)
 * with priority and lifetime for client-side rendering.
 */
class VisualFeedbackQueueSystem : public ecs::SingleComponentSystem<components::VisualFeedbackQueue> {
public:
    explicit VisualFeedbackQueueSystem(ecs::World* world);
    ~VisualFeedbackQueueSystem() override = default;

    std::string getName() const override { return "VisualFeedbackQueueSystem"; }

    bool createQueue(const std::string& entity_id);
    int queueEffect(const std::string& entity_id, int category, float intensity,
                    float duration, int priority, const std::string& label);
    bool removeEffect(const std::string& entity_id, int effect_id);
    int getActiveEffectCount(const std::string& entity_id) const;
    int getEffectsByCategory(const std::string& entity_id, int category) const;
    int getHighestPriority(const std::string& entity_id) const;
    int getTotalQueued(const std::string& entity_id) const;
    int getTotalExpired(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::VisualFeedbackQueue& vfq,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VISUAL_FEEDBACK_QUEUE_SYSTEM_H
