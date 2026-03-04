#ifndef NOVAFORGE_SYSTEMS_CREW_TRAINING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CREW_TRAINING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/crew_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Crew member training with skill progression over time
 *
 * Manages trainee enrollment, skill progress advancement, completion
 * tracking, and XP bonus multipliers for training rate.
 */
class CrewTrainingSystem : public ecs::SingleComponentSystem<components::CrewTraining> {
public:
    explicit CrewTrainingSystem(ecs::World* world);
    ~CrewTrainingSystem() override = default;

    std::string getName() const override { return "CrewTrainingSystem"; }

    bool initializeTraining(const std::string& entity_id);
    bool enrollTrainee(const std::string& entity_id, const std::string& trainee_id,
                       const std::string& skill_name);
    bool removeTrainee(const std::string& entity_id, const std::string& trainee_id);
    int getTraineeCount(const std::string& entity_id) const;
    float getProgress(const std::string& entity_id, const std::string& trainee_id) const;
    bool isComplete(const std::string& entity_id, const std::string& trainee_id) const;
    int getTotalCompleted(const std::string& entity_id) const;
    bool setXpBonus(const std::string& entity_id, float bonus);
    float getXpBonus(const std::string& entity_id) const;
    std::string getSkillName(const std::string& entity_id, const std::string& trainee_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CrewTraining& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CREW_TRAINING_SYSTEM_H
