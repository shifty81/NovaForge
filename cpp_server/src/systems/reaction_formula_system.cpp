#include "systems/reaction_formula_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ReactionFormulaSystem::ReactionFormulaSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ReactionFormulaSystem::updateComponent(ecs::Entity& entity,
    components::ReactionFormulaState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& job : comp.jobs) {
        if (job.completed || job.cancelled) continue;
        job.progress += delta_time * comp.efficiency;
        job.progress = (std::min)(job.progress, job.time_required);
        if (job.progress >= job.time_required) {
            job.completed = true;
            comp.total_completed++;
        }
    }
}

bool ReactionFormulaSystem::initialize(const std::string& entity_id,
    const std::string& facility_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ReactionFormulaState>();
    comp->facility_id = facility_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ReactionFormulaSystem::startReaction(const std::string& entity_id,
    const std::string& job_id, const std::string& formula_id,
    float time_required, const std::string& output_material,
    int output_quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    int active = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) active++;
    }
    if (active >= comp->max_concurrent_reactions) return false;

    components::ReactionFormulaState::ReactionJob job;
    job.job_id = job_id;
    job.formula_id = formula_id;
    job.time_required = time_required;
    job.output_material = output_material;
    job.output_quantity = output_quantity;
    comp->jobs.push_back(job);
    comp->total_started++;
    return true;
}

bool ReactionFormulaSystem::addInput(const std::string& entity_id,
    const std::string& job_id, const std::string& material_id, int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& job : comp->jobs) {
        if (job.job_id == job_id && !job.completed && !job.cancelled) {
            components::ReactionFormulaState::InputMaterial input;
            input.material_id = material_id;
            input.quantity = quantity;
            job.inputs.push_back(input);
            return true;
        }
    }
    return false;
}

bool ReactionFormulaSystem::cancelReaction(const std::string& entity_id,
    const std::string& job_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& job : comp->jobs) {
        if (job.job_id == job_id && !job.completed && !job.cancelled) {
            job.cancelled = true;
            return true;
        }
    }
    return false;
}

bool ReactionFormulaSystem::setEfficiency(const std::string& entity_id,
    float efficiency) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->efficiency = (std::max)(0.0f, efficiency);
    return true;
}

int ReactionFormulaSystem::getActiveReactionCount(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& j : comp->jobs) {
        if (!j.completed && !j.cancelled) count++;
    }
    return count;
}

bool ReactionFormulaSystem::isReactionCompleted(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.completed;
    }
    return false;
}

float ReactionFormulaSystem::getReactionProgress(const std::string& entity_id,
    const std::string& job_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& j : comp->jobs) {
        if (j.job_id == job_id) return j.progress;
    }
    return 0.0f;
}

int ReactionFormulaSystem::getTotalCompleted(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_completed : 0;
}

float ReactionFormulaSystem::getEfficiency(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->efficiency : 0.0f;
}

} // namespace systems
} // namespace atlas
