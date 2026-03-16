#ifndef NOVAFORGE_SYSTEMS_SKILL_PLAN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SKILL_PLAN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class SkillPlanSystem
    : public ecs::SingleComponentSystem<components::SkillPlanState> {
public:
    explicit SkillPlanSystem(ecs::World* world);
    ~SkillPlanSystem() override = default;

    std::string getName() const override { return "SkillPlanSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Plan management ---
    bool createPlan(const std::string& entity_id,
                    const std::string& plan_id,
                    const std::string& plan_name);
    bool deletePlan(const std::string& entity_id,
                    const std::string& plan_id);
    bool renamePlan(const std::string& entity_id,
                    const std::string& plan_id,
                    const std::string& new_name);
    bool clearPlans(const std::string& entity_id);

    // --- Plan activation ---
    bool activatePlan(const std::string& entity_id,
                      const std::string& plan_id);
    bool deactivatePlan(const std::string& entity_id);

    // --- Skill management within a plan ---
    bool addSkill(const std::string& entity_id,
                  const std::string& plan_id,
                  const std::string& skill_id,
                  const std::string& skill_name,
                  int target_level,
                  float training_time);
    bool removeSkill(const std::string& entity_id,
                     const std::string& plan_id,
                     const std::string& skill_id);
    bool clearSkills(const std::string& entity_id,
                     const std::string& plan_id);
    bool moveSkill(const std::string& entity_id,
                   const std::string& plan_id,
                   const std::string& skill_id,
                   int new_index);

    // --- Configuration ---
    bool setMaxPlans(const std::string& entity_id, int max_plans);

    // --- Queries ---
    int         getPlanCount(const std::string& entity_id) const;
    bool        hasPlan(const std::string& entity_id,
                        const std::string& plan_id) const;
    std::string getPlanName(const std::string& entity_id,
                            const std::string& plan_id) const;
    int         getSkillCount(const std::string& entity_id,
                              const std::string& plan_id) const;
    float       getTotalTrainingTime(const std::string& entity_id,
                                     const std::string& plan_id) const;
    std::string getActivePlanId(const std::string& entity_id) const;
    bool        isActivePlan(const std::string& entity_id,
                             const std::string& plan_id) const;
    int         getTotalPlansCreated(const std::string& entity_id) const;
    int         getTotalPlansDeleted(const std::string& entity_id) const;
    int         getTotalSkillsPlanned(const std::string& entity_id) const;
    bool        hasSkillInPlan(const std::string& entity_id,
                               const std::string& plan_id,
                               const std::string& skill_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SkillPlanState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SKILL_PLAN_SYSTEM_H
