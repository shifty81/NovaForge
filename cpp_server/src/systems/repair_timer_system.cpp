#include "systems/repair_timer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RepairTimerSystem::RepairTimerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void RepairTimerSystem::updateComponent(ecs::Entity& /*entity*/,
                                         components::RepairTimerState& comp,
                                         float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed) continue;
        if (job.ticks_remaining <= 0) continue;

        comp.total_repaired += job.amount_per_tick;
        job.ticks_elapsed++;
        job.ticks_remaining--;

        if (job.ticks_remaining == 0) {
            job.completed = true;
            comp.total_jobs_completed++;
        }
    }
}

bool RepairTimerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RepairTimerState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool RepairTimerSystem::addJob(const std::string& entity_id,
                                const std::string& job_id,
                                components::RepairTimerState::RepairLayer layer,
                                float amount_per_tick,
                                int   ticks_remaining) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (job_id.empty()) return false;
    if (amount_per_tick <= 0.0f) return false;
    if (ticks_remaining <= 0) return false;
    if (static_cast<int>(comp->jobs.size()) >= comp->max_jobs) return false;

    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
                           [&](const components::RepairTimerState::RepairJob& j) {
                               return j.job_id == job_id;
                           });
    if (it != comp->jobs.end()) return false;

    components::RepairTimerState::RepairJob job;
    job.job_id          = job_id;
    job.layer           = layer;
    job.amount_per_tick = amount_per_tick;
    job.ticks_remaining = ticks_remaining;
    comp->jobs.push_back(job);
    comp->total_jobs_started++;
    return true;
}

bool RepairTimerSystem::cancelJob(const std::string& entity_id,
                                   const std::string& job_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
                           [&](const components::RepairTimerState::RepairJob& j) {
                               return j.job_id == job_id;
                           });
    if (it == comp->jobs.end()) return false;
    comp->jobs.erase(it);
    return true;
}

bool RepairTimerSystem::clearJobs(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->jobs.clear();
    return true;
}

int RepairTimerSystem::getJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->jobs.size());
}

int RepairTimerSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs)
        if (!j.completed) count++;
    return count;
}

bool RepairTimerSystem::isJobActive(const std::string& entity_id,
                                     const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
                           [&](const components::RepairTimerState::RepairJob& j) {
                               return j.job_id == job_id;
                           });
    if (it == comp->jobs.end()) return false;
    return !it->completed;
}

bool RepairTimerSystem::isJobComplete(const std::string& entity_id,
                                       const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
                           [&](const components::RepairTimerState::RepairJob& j) {
                               return j.job_id == job_id;
                           });
    if (it == comp->jobs.end()) return false;
    return it->completed;
}

bool RepairTimerSystem::hasJob(const std::string& entity_id,
                                const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return std::find_if(comp->jobs.begin(), comp->jobs.end(),
                        [&](const components::RepairTimerState::RepairJob& j) {
                            return j.job_id == job_id;
                        }) != comp->jobs.end();
}

int RepairTimerSystem::getTicksRemaining(const std::string& entity_id,
                                          const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    auto it = std::find_if(comp->jobs.begin(), comp->jobs.end(),
                           [&](const components::RepairTimerState::RepairJob& j) {
                               return j.job_id == job_id;
                           });
    if (it == comp->jobs.end()) return 0;
    return it->ticks_remaining;
}

float RepairTimerSystem::getTotalRepaired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_repaired;
}

int RepairTimerSystem::getTotalJobsStarted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_jobs_started;
}

int RepairTimerSystem::getTotalJobsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_jobs_completed;
}

float RepairTimerSystem::getRepairByLayer(
        const std::string& entity_id,
        components::RepairTimerState::RepairLayer layer) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& job : comp->jobs) {
        if (job.layer == layer) {
            int ticks_done = job.ticks_elapsed;
            total += job.amount_per_tick * static_cast<float>(ticks_done);
        }
    }
    return total;
}

} // namespace systems
} // namespace atlas
