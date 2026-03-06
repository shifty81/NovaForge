#include "systems/ore_processing_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

OreProcessingSystem::OreProcessingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool OreProcessingSystem::initializeProcessing(const std::string& entity_id,
                                               float efficiency,
                                               int max_jobs) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::OreProcessing>()) return false;

    auto proc = std::make_unique<components::OreProcessing>();
    proc->efficiency = efficiency;
    proc->max_concurrent_jobs = max_jobs;
    entity->addComponent(std::move(proc));
    return true;
}

bool OreProcessingSystem::queueOre(const std::string& entity_id,
                                   const std::string& ore_type,
                                   float amount,
                                   float processing_time) {
    auto* proc = getComponentFor(entity_id);
    if (!proc || !proc->active) return false;
    if (ore_type.empty() || amount <= 0.0f || processing_time <= 0.0f) return false;

    // Count active (incomplete) jobs
    int active = 0;
    for (const auto& j : proc->jobs) {
        if (!j.completed) ++active;
    }
    if (active >= proc->max_concurrent_jobs) return false;

    components::OreProcessing::OreJob job;
    job.ore_type = ore_type;
    job.raw_amount = amount;
    job.processing_time = processing_time;
    proc->jobs.push_back(job);
    return true;
}

int OreProcessingSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* proc = getComponentFor(entity_id);
    if (!proc) return 0;
    int count = 0;
    for (const auto& j : proc->jobs) {
        if (!j.completed) ++count;
    }
    return count;
}

float OreProcessingSystem::getTotalRefined(const std::string& entity_id) const {
    auto* proc = getComponentFor(entity_id);
    return proc ? proc->total_refined_output : 0.0f;
}

int OreProcessingSystem::getBatchesCompleted(const std::string& entity_id) const {
    auto* proc = getComponentFor(entity_id);
    return proc ? proc->total_batches_completed : 0;
}

bool OreProcessingSystem::setEfficiency(const std::string& entity_id, float efficiency) {
    auto* proc = getComponentFor(entity_id);
    if (!proc) return false;
    if (efficiency < 0.0f || efficiency > 1.0f) return false;
    proc->efficiency = efficiency;
    return true;
}

bool OreProcessingSystem::setProcessingSpeed(const std::string& entity_id, float speed) {
    auto* proc = getComponentFor(entity_id);
    if (!proc) return false;
    if (speed <= 0.0f) return false;
    proc->processing_speed = speed;
    return true;
}

void OreProcessingSystem::updateComponent(ecs::Entity& /*entity*/,
                                          components::OreProcessing& proc,
                                          float delta_time) {
    if (!proc.active) return;
    proc.elapsed += delta_time;

    for (auto& job : proc.jobs) {
        if (job.completed) continue;

        float advance = (delta_time / job.processing_time) * proc.processing_speed;
        job.progress += advance;

        if (job.progress >= 1.0f) {
            job.progress = 1.0f;
            job.completed = true;
            job.refined_amount = job.raw_amount * proc.efficiency;
            proc.total_raw_processed += job.raw_amount;
            proc.total_refined_output += job.refined_amount;
            proc.total_batches_completed++;
        }
    }
}

} // namespace systems
} // namespace atlas
