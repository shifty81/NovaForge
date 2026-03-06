#ifndef NOVAFORGE_SYSTEMS_SKILL_QUEUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SKILL_QUEUE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages player skill training queues with time-based progression
 *
 * Players queue skills for training; the system advances training time each
 * tick, completes skills when their duration is reached, and auto-starts
 * the next queued skill. Supports pause/resume, reordering, and level-based
 * training time multipliers. Completed skills are recorded for progression.
 */
class SkillQueueSystem : public ecs::SingleComponentSystem<components::SkillQueueState> {
public:
    explicit SkillQueueSystem(ecs::World* world);
    ~SkillQueueSystem() override = default;

    std::string getName() const override { return "SkillQueueSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Queue management
    bool enqueueSkill(const std::string& entity_id, const std::string& skill_id,
                      int target_level, float train_duration);
    bool dequeueSkill(const std::string& entity_id, const std::string& skill_id);
    int getQueueLength(const std::string& entity_id) const;
    bool isQueued(const std::string& entity_id, const std::string& skill_id) const;

    // Current training
    std::string getCurrentSkillId(const std::string& entity_id) const;
    float getCurrentProgress(const std::string& entity_id) const;
    float getRemainingTime(const std::string& entity_id) const;
    bool isTraining(const std::string& entity_id) const;

    // Pause / resume
    bool pauseTraining(const std::string& entity_id);
    bool resumeTraining(const std::string& entity_id);
    bool isPaused(const std::string& entity_id) const;

    // Completed skills
    int getCompletedCount(const std::string& entity_id) const;
    bool hasCompleted(const std::string& entity_id, const std::string& skill_id) const;
    int getSkillLevel(const std::string& entity_id, const std::string& skill_id) const;

    // Reorder
    bool moveToFront(const std::string& entity_id, const std::string& skill_id);

    // Total time
    float getTotalTrainingTime(const std::string& entity_id) const;
    float getTotalRemainingTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SkillQueueState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SKILL_QUEUE_SYSTEM_H
