#ifndef NOVAFORGE_SYSTEMS_SKILL_TRAINING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SKILL_TRAINING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Skill training queue — SP accrual, level completion, training time
 *
 * Manages a player's skill training queue.  Skills have 5 levels, each
 * requiring progressively more SP (base × level²).  The active skill in
 * the queue accrues SP each tick at a configurable rate.  When sufficient
 * SP is accumulated the skill levels up and training moves to the next
 * queue entry.  Supports queue reordering, skill removal, and total-SP
 * tracking for analytics.
 */
class SkillTrainingSystem : public ecs::SingleComponentSystem<components::SkillTrainingState> {
public:
    explicit SkillTrainingSystem(ecs::World* world);
    ~SkillTrainingSystem() override = default;

    std::string getName() const override { return "SkillTrainingSystem"; }

public:
    bool initialize(const std::string& entity_id, float sp_per_second);
    bool enqueueSkill(const std::string& entity_id, const std::string& skill_id,
                      int target_level, int base_sp_cost);
    bool removeSkill(const std::string& entity_id, const std::string& skill_id);
    bool pauseTraining(const std::string& entity_id);
    bool resumeTraining(const std::string& entity_id);

    int getQueueLength(const std::string& entity_id) const;
    std::string getCurrentSkillId(const std::string& entity_id) const;
    int getCurrentSkillLevel(const std::string& entity_id) const;
    float getCurrentSkillProgress(const std::string& entity_id) const;
    int getTotalSkillsCompleted(const std::string& entity_id) const;
    float getTotalSpEarned(const std::string& entity_id) const;
    bool isTraining(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SkillTrainingState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SKILL_TRAINING_SYSTEM_H
