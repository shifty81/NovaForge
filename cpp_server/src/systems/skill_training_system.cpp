#include "systems/skill_training_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SkillTrainingSystem::SkillTrainingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SkillTrainingSystem::updateComponent(ecs::Entity& entity,
    components::SkillTrainingState& comp, float delta_time) {
    if (!comp.active) return;
    if (comp.paused) return;
    if (comp.queue.empty()) return;
    comp.elapsed += delta_time;

    // Accrue SP to the front skill in the queue
    auto& front = comp.queue.front();
    float sp_gain = comp.sp_per_second * delta_time;
    front.accumulated_sp += sp_gain;
    comp.total_sp_earned += sp_gain;

    // SP required for target level: base_sp_cost × target_level²
    float required = static_cast<float>(front.base_sp_cost) *
                     static_cast<float>(front.target_level * front.target_level);

    if (front.accumulated_sp >= required) {
        front.current_level = front.target_level;
        front.completed = true;
        comp.total_skills_completed++;

        // Remove completed skill from queue
        comp.queue.erase(comp.queue.begin());
    }
}

bool SkillTrainingSystem::initialize(const std::string& entity_id, float sp_per_second) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (sp_per_second <= 0.0f) return false;

    auto comp = std::make_unique<components::SkillTrainingState>();
    comp->sp_per_second = sp_per_second;
    entity->addComponent(std::move(comp));
    return true;
}

bool SkillTrainingSystem::enqueueSkill(const std::string& entity_id,
    const std::string& skill_id, int target_level, int base_sp_cost) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_level < 1 || target_level > 5) return false;
    if (base_sp_cost <= 0) return false;

    // No duplicate skill IDs in queue
    for (const auto& entry : comp->queue) {
        if (entry.skill_id == skill_id) return false;
    }
    if (static_cast<int>(comp->queue.size()) >= comp->max_queue_size) return false;

    components::SkillTrainingState::SkillEntry entry;
    entry.skill_id = skill_id;
    entry.target_level = target_level;
    entry.base_sp_cost = base_sp_cost;
    comp->queue.push_back(entry);
    return true;
}

bool SkillTrainingSystem::removeSkill(const std::string& entity_id,
    const std::string& skill_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->queue.begin(), comp->queue.end(),
        [&skill_id](const components::SkillTrainingState::SkillEntry& e) {
            return e.skill_id == skill_id;
        });
    if (it == comp->queue.end()) return false;
    comp->queue.erase(it);
    return true;
}

bool SkillTrainingSystem::pauseTraining(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->paused) return false;
    comp->paused = true;
    return true;
}

bool SkillTrainingSystem::resumeTraining(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->paused) return false;
    comp->paused = false;
    return true;
}

int SkillTrainingSystem::getQueueLength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->queue.size()) : 0;
}

std::string SkillTrainingSystem::getCurrentSkillId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->queue.empty()) return "";
    return comp->queue.front().skill_id;
}

int SkillTrainingSystem::getCurrentSkillLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->queue.empty()) return 0;
    return comp->queue.front().current_level;
}

float SkillTrainingSystem::getCurrentSkillProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->queue.empty()) return 0.0f;

    const auto& front = comp->queue.front();
    float required = static_cast<float>(front.base_sp_cost) *
                     static_cast<float>(front.target_level * front.target_level);
    if (required <= 0.0f) return 0.0f;
    return std::min(1.0f, front.accumulated_sp / required);
}

int SkillTrainingSystem::getTotalSkillsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_skills_completed : 0;
}

float SkillTrainingSystem::getTotalSpEarned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_sp_earned : 0.0f;
}

bool SkillTrainingSystem::isTraining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return !comp->paused && !comp->queue.empty();
}

} // namespace systems
} // namespace atlas
