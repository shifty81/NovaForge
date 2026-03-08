#include "systems/salvage_processing_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SalvageProcessingSystem::SalvageProcessingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SalvageProcessingSystem::updateComponent(ecs::Entity& /*entity*/,
    components::SalvageProcessingState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed) continue;

        job.progress += (comp.processing_speed + comp.skill_bonus) * delta_time / job.processing_time;
        if (job.progress >= 1.0f) {
            job.progress = 1.0f;
            job.completed = true;
            if (job.success_chance + comp.skill_bonus >= 0.5f) {
                job.successful = true;
                comp.total_materials_salvaged += job.yield_amount;
                comp.total_jobs_completed++;
            } else {
                job.successful = false;
                comp.total_jobs_failed++;
            }
        }
    }
}

bool SalvageProcessingSystem::initialize(const std::string& entity_id, float processing_speed) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (processing_speed <= 0.0f) return false;

    auto comp = std::make_unique<components::SalvageProcessingState>();
    comp->processing_speed = processing_speed;
    entity->addComponent(std::move(comp));
    return true;
}

bool SalvageProcessingSystem::addJob(const std::string& entity_id,
    const std::string& wreck_id, const std::string& material_type,
    float processing_time, float yield_amount, float success_chance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (wreck_id.empty()) return false;
    if (material_type.empty()) return false;
    if (processing_time <= 0.0f) return false;
    if (yield_amount <= 0.0f) return false;
    if (success_chance < 0.0f || success_chance > 1.0f) return false;

    for (const auto& j : comp->jobs) {
        if (j.wreck_id == wreck_id) return false;
    }
    if (static_cast<int>(comp->jobs.size()) >= comp->max_jobs) return false;

    components::SalvageProcessingState::SalvageJob job;
    job.wreck_id = wreck_id;
    job.material_type = material_type;
    job.processing_time = processing_time;
    job.yield_amount = yield_amount;
    job.success_chance = success_chance;
    comp->jobs.push_back(job);
    return true;
}

bool SalvageProcessingSystem::removeJob(const std::string& entity_id,
    const std::string& wreck_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
        [&wreck_id](const components::SalvageProcessingState::SalvageJob& j) {
            return j.wreck_id == wreck_id;
        });
    if (it == comp->jobs.end()) return false;
    comp->jobs.erase(it);
    return true;
}

bool SalvageProcessingSystem::setProcessingSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed <= 0.0f) return false;
    comp->processing_speed = speed;
    return true;
}

bool SalvageProcessingSystem::setSkillBonus(const std::string& entity_id, float bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bonus < 0.0f) return false;
    comp->skill_bonus = bonus;
    return true;
}

int SalvageProcessingSystem::getJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->jobs.size()) : 0;
}

int SalvageProcessingSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed) count++;
    }
    return count;
}

float SalvageProcessingSystem::getJobProgress(const std::string& entity_id,
    const std::string& wreck_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& j : comp->jobs) {
        if (j.wreck_id == wreck_id) return j.progress;
    }
    return 0.0f;
}

bool SalvageProcessingSystem::isJobCompleted(const std::string& entity_id,
    const std::string& wreck_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.wreck_id == wreck_id) return j.completed;
    }
    return false;
}

bool SalvageProcessingSystem::isJobSuccessful(const std::string& entity_id,
    const std::string& wreck_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.wreck_id == wreck_id) return j.successful;
    }
    return false;
}

float SalvageProcessingSystem::getTotalMaterialsSalvaged(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_materials_salvaged : 0.0f;
}

int SalvageProcessingSystem::getTotalJobsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jobs_completed : 0;
}

int SalvageProcessingSystem::getTotalJobsFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jobs_failed : 0;
}

float SalvageProcessingSystem::getProcessingSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->processing_speed : 0.0f;
}

float SalvageProcessingSystem::getSkillBonus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->skill_bonus : 0.0f;
}

} // namespace systems
} // namespace atlas
