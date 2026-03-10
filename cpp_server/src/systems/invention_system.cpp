#include "systems/invention_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

InventionSystem::InventionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void InventionSystem::updateComponent(ecs::Entity& entity,
    components::InventionState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed || job.cancelled) continue;
        job.progress += delta_time * comp.research_speed;
        if (job.progress >= job.time_required) {
            job.progress = job.time_required;
            job.completed = true;
            comp.total_attempted++;
            // Determine success: use deterministic check based on chance
            float effective_chance = job.base_chance + job.skill_bonus;
            if (effective_chance >= 1.0f) {
                job.succeeded = true;
                comp.total_succeeded++;
            } else if (effective_chance <= 0.0f) {
                job.succeeded = false;
                comp.total_failed++;
            } else {
                // Simple deterministic success for testability:
                // success if effective chance > 0.5, otherwise fail
                // In production this would use a PRNG seeded per-job
                job.succeeded = (effective_chance > 0.5f);
                if (job.succeeded) {
                    comp.total_succeeded++;
                    job.t2_blueprint_id = job.t1_blueprint_id + "_t2";
                } else {
                    comp.total_failed++;
                }
            }
        }
    }
}

bool InventionSystem::initialize(const std::string& entity_id,
    const std::string& facility_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::InventionState>();
    comp->facility_id = facility_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool InventionSystem::startJob(const std::string& entity_id,
    const std::string& job_id, const std::string& t1_blueprint_id,
    const std::string& datacore_1, const std::string& datacore_2,
    float base_chance, float time_required) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    // Check concurrent limit (count active only)
    int active = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) active++;
    }
    if (active >= comp->max_concurrent_jobs) return false;

    components::InventionState::InventionJob job;
    job.job_id = job_id;
    job.t1_blueprint_id = t1_blueprint_id;
    job.datacore_1 = datacore_1;
    job.datacore_2 = datacore_2;
    job.base_chance = base_chance;
    job.time_required = time_required;
    comp->jobs.push_back(job);
    return true;
}

bool InventionSystem::cancelJob(const std::string& entity_id,
    const std::string& job_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& job : comp->jobs) {
        if (job.job_id == job_id && !job.completed && !job.cancelled) {
            job.cancelled = true;
            comp->total_cancelled++;
            return true;
        }
    }
    return false;
}

bool InventionSystem::setResearchSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->research_speed = speed;
    return true;
}

int InventionSystem::getJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->jobs.size()) : 0;
}

int InventionSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) count++;
    }
    return count;
}

int InventionSystem::getTotalAttempted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_attempted : 0;
}

int InventionSystem::getTotalSucceeded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_succeeded : 0;
}

int InventionSystem::getTotalFailed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_failed : 0;
}

int InventionSystem::getTotalCancelled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cancelled : 0;
}

float InventionSystem::getJobProgress(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.progress;
    }
    return 0.0f;
}

bool InventionSystem::isJobCompleted(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.completed;
    }
    return false;
}

float InventionSystem::getResearchSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->research_speed : 0.0f;
}

} // namespace systems
} // namespace atlas
