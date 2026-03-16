#include "systems/skill_plan_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SkillPlanSystem::SkillPlanSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void SkillPlanSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SkillPlanState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SkillPlanSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SkillPlanState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Plan management
// ---------------------------------------------------------------------------

bool SkillPlanSystem::createPlan(const std::string& entity_id,
                                  const std::string& plan_id,
                                  const std::string& plan_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (plan_id.empty()) return false;
    if (plan_name.empty()) return false;

    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) return false;
    }

    if (static_cast<int>(comp->plans.size()) >= comp->max_plans)
        return false;

    components::SkillPlanState::SkillPlan plan;
    plan.plan_id   = plan_id;
    plan.plan_name = plan_name;
    comp->plans.push_back(plan);
    comp->total_plans_created++;
    return true;
}

bool SkillPlanSystem::deletePlan(const std::string& entity_id,
                                  const std::string& plan_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->plans.begin(), comp->plans.end(),
        [&](const components::SkillPlanState::SkillPlan& p) {
            return p.plan_id == plan_id;
        });
    if (it == comp->plans.end()) return false;
    comp->plans.erase(it);
    comp->total_plans_deleted++;
    if (comp->active_plan_id == plan_id) {
        comp->active_plan_id.clear();
    }
    return true;
}

bool SkillPlanSystem::renamePlan(const std::string& entity_id,
                                  const std::string& plan_id,
                                  const std::string& new_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_name.empty()) return false;
    for (auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            p.plan_name = new_name;
            return true;
        }
    }
    return false;
}

bool SkillPlanSystem::clearPlans(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->plans.clear();
    comp->active_plan_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Plan activation
// ---------------------------------------------------------------------------

bool SkillPlanSystem::activatePlan(const std::string& entity_id,
                                    const std::string& plan_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    bool found = false;
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) { found = true; break; }
    }
    if (!found) return false;
    comp->active_plan_id = plan_id;
    return true;
}

bool SkillPlanSystem::deactivatePlan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->active_plan_id.empty()) return false;
    comp->active_plan_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Skill management within a plan
// ---------------------------------------------------------------------------

bool SkillPlanSystem::addSkill(const std::string& entity_id,
                                const std::string& plan_id,
                                const std::string& skill_id,
                                const std::string& skill_name,
                                int target_level,
                                float training_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (skill_id.empty()) return false;
    if (skill_name.empty()) return false;
    if (target_level < 1 || target_level > 5) return false;
    if (training_time < 0.0f) return false;

    for (auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            for (const auto& s : p.skills) {
                if (s.skill_id == skill_id) return false;
            }
            components::SkillPlanState::PlannedSkill skill;
            skill.skill_id      = skill_id;
            skill.skill_name    = skill_name;
            skill.target_level  = target_level;
            skill.training_time = training_time;
            p.skills.push_back(skill);
            comp->total_skills_planned++;
            return true;
        }
    }
    return false;
}

bool SkillPlanSystem::removeSkill(const std::string& entity_id,
                                   const std::string& plan_id,
                                   const std::string& skill_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            auto it = std::find_if(p.skills.begin(), p.skills.end(),
                [&](const components::SkillPlanState::PlannedSkill& s) {
                    return s.skill_id == skill_id;
                });
            if (it == p.skills.end()) return false;
            p.skills.erase(it);
            return true;
        }
    }
    return false;
}

bool SkillPlanSystem::clearSkills(const std::string& entity_id,
                                   const std::string& plan_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            p.skills.clear();
            return true;
        }
    }
    return false;
}

bool SkillPlanSystem::moveSkill(const std::string& entity_id,
                                 const std::string& plan_id,
                                 const std::string& skill_id,
                                 int new_index) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_index < 0) return false;

    for (auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            auto it = std::find_if(p.skills.begin(), p.skills.end(),
                [&](const components::SkillPlanState::PlannedSkill& s) {
                    return s.skill_id == skill_id;
                });
            if (it == p.skills.end()) return false;

            int old_index = static_cast<int>(std::distance(p.skills.begin(), it));
            if (new_index >= static_cast<int>(p.skills.size()))
                new_index = static_cast<int>(p.skills.size()) - 1;
            if (new_index == old_index) return true;

            auto skill = *it;
            p.skills.erase(it);
            p.skills.insert(p.skills.begin() + new_index, skill);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool SkillPlanSystem::setMaxPlans(const std::string& entity_id,
                                   int max_plans) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_plans <= 0) return false;
    comp->max_plans = max_plans;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int SkillPlanSystem::getPlanCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->plans.size()) : 0;
}

bool SkillPlanSystem::hasPlan(const std::string& entity_id,
                               const std::string& plan_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) return true;
    }
    return false;
}

std::string SkillPlanSystem::getPlanName(const std::string& entity_id,
                                          const std::string& plan_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) return p.plan_name;
    }
    return "";
}

int SkillPlanSystem::getSkillCount(const std::string& entity_id,
                                    const std::string& plan_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) return static_cast<int>(p.skills.size());
    }
    return 0;
}

float SkillPlanSystem::getTotalTrainingTime(const std::string& entity_id,
                                             const std::string& plan_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            float total = 0.0f;
            for (const auto& s : p.skills) total += s.training_time;
            return total;
        }
    }
    return 0.0f;
}

std::string SkillPlanSystem::getActivePlanId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_plan_id : "";
}

bool SkillPlanSystem::isActivePlan(const std::string& entity_id,
                                    const std::string& plan_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->active_plan_id == plan_id;
}

int SkillPlanSystem::getTotalPlansCreated(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_plans_created : 0;
}

int SkillPlanSystem::getTotalPlansDeleted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_plans_deleted : 0;
}

int SkillPlanSystem::getTotalSkillsPlanned(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_skills_planned : 0;
}

bool SkillPlanSystem::hasSkillInPlan(const std::string& entity_id,
                                      const std::string& plan_id,
                                      const std::string& skill_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->plans) {
        if (p.plan_id == plan_id) {
            for (const auto& s : p.skills) {
                if (s.skill_id == skill_id) return true;
            }
            return false;
        }
    }
    return false;
}

} // namespace systems
} // namespace atlas
