#include "systems/blueprint_research_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

BlueprintResearchSystem::BlueprintResearchSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void BlueprintResearchSystem::updateComponent(ecs::Entity& entity,
    components::BlueprintResearchState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed || job.cancelled) continue;
        job.progress += delta_time * comp.research_speed;
        if (job.progress >= job.time_required) {
            job.progress = job.time_required;
            job.completed = true;
            job.current_level = job.target_level;
            comp.total_completed++;
        }
    }

    // Remove completed and cancelled jobs
    comp.jobs.erase(
        std::remove_if(comp.jobs.begin(), comp.jobs.end(),
            [](const components::BlueprintResearchState::ResearchJob& j) {
                return j.completed || j.cancelled;
            }),
        comp.jobs.end());
}

bool BlueprintResearchSystem::initialize(const std::string& entity_id,
    const std::string& facility_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::BlueprintResearchState>();
    comp->facility_id = facility_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool BlueprintResearchSystem::startResearch(const std::string& entity_id,
    const std::string& blueprint_id,
    components::BlueprintResearchState::ResearchType type,
    int current_level, int target_level, float time_required) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Validate level bounds
    int max_level = (type == components::BlueprintResearchState::ResearchType::MaterialEfficiency)
        ? components::BlueprintResearchState::MAX_ME_LEVEL
        : components::BlueprintResearchState::MAX_TE_LEVEL;
    if (target_level > max_level || target_level <= current_level) return false;

    // Check concurrent job limit
    int active = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) active++;
    }
    if (active >= comp->max_concurrent_jobs) return false;

    // Check for duplicate blueprint research
    for (const auto& j : comp->jobs) {
        if (j.blueprint_id == blueprint_id && !j.completed && !j.cancelled) return false;
    }

    components::BlueprintResearchState::ResearchJob job;
    job.blueprint_id = blueprint_id;
    job.type = type;
    job.current_level = current_level;
    job.target_level = target_level;
    job.time_required = time_required;
    comp->jobs.push_back(job);
    return true;
}

bool BlueprintResearchSystem::cancelResearch(const std::string& entity_id,
    const std::string& blueprint_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& j : comp->jobs) {
        if (j.blueprint_id == blueprint_id && !j.completed && !j.cancelled) {
            j.cancelled = true;
            comp->total_cancelled++;
            return true;
        }
    }
    return false;
}

bool BlueprintResearchSystem::setResearchSpeed(const std::string& entity_id,
    float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->research_speed = speed;
    return true;
}

int BlueprintResearchSystem::getJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->jobs.size()) : 0;
}

int BlueprintResearchSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) count++;
    }
    return count;
}

int BlueprintResearchSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_completed : 0;
}

int BlueprintResearchSystem::getTotalCancelled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cancelled : 0;
}

float BlueprintResearchSystem::getJobProgress(const std::string& entity_id,
    const std::string& blueprint_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& j : comp->jobs) {
        if (j.blueprint_id == blueprint_id) return j.progress;
    }
    return 0.0f;
}

bool BlueprintResearchSystem::isJobCompleted(const std::string& entity_id,
    const std::string& blueprint_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.blueprint_id == blueprint_id) return j.completed;
    }
    return false;
}

float BlueprintResearchSystem::getResearchSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->research_speed : 0.0f;
}

} // namespace systems
} // namespace atlas
