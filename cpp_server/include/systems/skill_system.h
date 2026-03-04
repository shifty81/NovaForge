#ifndef NOVAFORGE_SYSTEMS_SKILL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SKILL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes skill training queues and applies completion
 *
 * Each tick:
 *  - Decrements time_remaining on the front of the training queue
 *  - On completion, levels up the skill and pops from queue
 *  - Accumulates total SP
 */
class SkillSystem : public ecs::SingleComponentSystem<components::SkillSet> {
public:
    explicit SkillSystem(ecs::World* world);
    ~SkillSystem() override = default;

    std::string getName() const override { return "SkillSystem"; }

    /**
     * @brief Add a skill to the training queue
     * @return true if queued successfully
     */
    bool queueSkillTraining(const std::string& entity_id,
                            const std::string& skill_id,
                            const std::string& skill_name,
                            int target_level,
                            float training_time,
                            float multiplier = 1.0f);

    /**
     * @brief Instantly train a skill to a level (for testing/admin)
     */
    bool trainSkillInstant(const std::string& entity_id,
                           const std::string& skill_id,
                           const std::string& skill_name,
                           int level);

    /**
     * @brief Get the level of a specific skill for an entity
     * @return skill level (0 if not trained)
     */
    int getSkillLevel(const std::string& entity_id,
                      const std::string& skill_id);

protected:
    void updateComponent(ecs::Entity& entity, components::SkillSet& skillset, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SKILL_SYSTEM_H
