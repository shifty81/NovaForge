#include "systems/capital_component_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CapitalComponentSystem::CapitalComponentSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CapitalComponentSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CapitalComponentState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed || job.cancelled) continue;

        float effective_time = job.time_per_run * (float)job.runs *
                               (1.0f - job.te_bonus);
        if (effective_time <= 0.0f) effective_time = 1.0f;

        job.progress += delta_time / effective_time;
        if (job.progress >= 1.0f) {
            job.progress = 1.0f;
            job.completed = true;
            job.units_produced = job.runs;
            comp.total_completed++;
            comp.total_units_produced += job.units_produced;
        }
    }
}

bool CapitalComponentSystem::initialize(const std::string& entity_id,
    const std::string& facility_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CapitalComponentState>();
    comp->facility_id = facility_id;
    entity->addComponent(std::move(comp));
    return true;
}

std::string CapitalComponentSystem::startJob(const std::string& entity_id,
    const std::string& component_type, const std::string& blueprint_id,
    int runs, float me_bonus, float te_bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    if (runs <= 0) return "";

    // Count active jobs
    int active = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) active++;
    }
    if (active >= comp->max_concurrent_jobs) return "";

    components::CapitalComponentState::CapitalJob job;
    job.job_id = "capjob_" + std::to_string(++job_counter_);
    job.component_type = component_type;
    job.blueprint_id = blueprint_id;
    job.runs = runs;
    job.me_bonus = std::max(0.0f, std::min(1.0f, me_bonus));
    job.te_bonus = std::max(0.0f, std::min(1.0f, te_bonus));
    comp->jobs.push_back(job);
    return job.job_id;
}

bool CapitalComponentSystem::cancelJob(const std::string& entity_id,
    const std::string& job_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& j : comp->jobs) {
        if (j.job_id == job_id && !j.completed && !j.cancelled) {
            j.cancelled = true;
            return true;
        }
    }
    return false;
}

int CapitalComponentSystem::getActiveJobCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) count++;
    }
    return count;
}

int CapitalComponentSystem::getCompletedJobCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_completed : 0;
}

int CapitalComponentSystem::getTotalUnitsProduced(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_units_produced : 0;
}

float CapitalComponentSystem::getJobProgress(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.progress;
    }
    return 0.0f;
}

bool CapitalComponentSystem::isJobComplete(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.completed;
    }
    return false;
}

} // namespace systems
} // namespace atlas
