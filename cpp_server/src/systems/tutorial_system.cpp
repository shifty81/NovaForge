#include "systems/tutorial_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TutorialSystem::TutorialSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void TutorialSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::TutorialState& comp,
                                      float delta_time) {
    if (!comp.active) return;
    if (!comp.is_active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool TutorialSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TutorialState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Step management
// ---------------------------------------------------------------------------

bool TutorialSystem::addStep(const std::string& entity_id,
                               const std::string& step_id,
                               const std::string& description) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (step_id.empty()) return false;
    if (comp->is_active || comp->is_complete) return false;

    for (const auto& s : comp->steps) {
        if (s.step_id == step_id) return false;
    }

    components::TutorialState::TutorialStep step;
    step.step_id     = step_id;
    step.description = description;
    comp->steps.push_back(step);
    return true;
}

bool TutorialSystem::startTutorial(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_active || comp->is_complete) return false;
    if (comp->steps.empty()) return false;
    comp->is_active = true;
    comp->current_step_index = 0;
    return true;
}

bool TutorialSystem::completeStep(const std::string& entity_id,
                                   const std::string& step_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_active || comp->is_complete) return false;
    if (comp->steps.empty()) return false;
    if (comp->current_step_index >= static_cast<int>(comp->steps.size())) return false;

    auto& current = comp->steps[comp->current_step_index];
    if (current.step_id != step_id) return false;
    if (current.completed) return false;

    current.completed = true;
    comp->completed_step_count++;
    comp->current_step_index++;

    if (comp->current_step_index >= static_cast<int>(comp->steps.size())) {
        comp->is_complete = true;
        comp->is_active   = false;
    }
    return true;
}

bool TutorialSystem::skipTutorial(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_active) return false;
    for (auto& s : comp->steps) {
        if (!s.completed) {
            s.completed = true;
            comp->completed_step_count++;
        }
    }
    comp->current_step_index = static_cast<int>(comp->steps.size());
    comp->is_skipped  = true;
    comp->is_complete = true;
    comp->is_active   = false;
    return true;
}

bool TutorialSystem::resetTutorial(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->steps) {
        s.completed = false;
    }
    comp->is_active         = false;
    comp->is_complete       = false;
    comp->is_skipped        = false;
    comp->current_step_index = 0;
    comp->completed_step_count = 0;
    comp->elapsed           = 0.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool TutorialSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->is_active;
}

bool TutorialSystem::isComplete(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->is_complete;
}

std::string TutorialSystem::getCurrentStepId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->is_active) return std::string();
    if (comp->current_step_index >= static_cast<int>(comp->steps.size())) return std::string();
    return comp->steps[comp->current_step_index].step_id;
}

int TutorialSystem::getCompletedStepCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->completed_step_count : 0;
}

int TutorialSystem::getTotalStepCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->steps.size()) : 0;
}

float TutorialSystem::getElapsed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->elapsed : 0.0f;
}

} // namespace systems
} // namespace atlas
