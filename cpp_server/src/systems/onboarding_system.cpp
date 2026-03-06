#include "systems/onboarding_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using OBS = components::OnboardingState;
}

OnboardingSystem::OnboardingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OnboardingSystem::updateComponent(ecs::Entity& entity,
    components::OnboardingState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool OnboardingSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OnboardingState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool OnboardingSystem::startTutorial(const std::string& entity_id, float start_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->current_phase != OBS::TutorialPhase::NotStarted) return false;
    state->current_phase = OBS::TutorialPhase::Welcome;
    state->start_time = start_time;
    return true;
}

bool OnboardingSystem::advancePhase(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->tutorial_complete || state->tutorial_skipped) return false;
    int current = static_cast<int>(state->current_phase);
    int completed = static_cast<int>(OBS::TutorialPhase::Completed);
    if (current >= completed) return false;
    state->current_phase = static_cast<OBS::TutorialPhase>(current + 1);
    if (state->current_phase == OBS::TutorialPhase::Completed) {
        state->tutorial_complete = true;
        state->completion_time = state->elapsed_time;
    }
    return true;
}

bool OnboardingSystem::skipTutorial(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->tutorial_complete || state->tutorial_skipped) return false;
    state->tutorial_skipped = true;
    state->current_phase = OBS::TutorialPhase::Completed;
    return true;
}

int OnboardingSystem::getCurrentPhase(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->current_phase) : -1;
}

bool OnboardingSystem::addObjective(const std::string& entity_id,
    const std::string& objective_id, const std::string& description, int phase) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->objectives.size()) >= state->max_objectives) return false;
    for (const auto& o : state->objectives) {
        if (o.objective_id == objective_id) return false;
    }
    OBS::Objective obj;
    obj.objective_id = objective_id;
    obj.description = description;
    obj.phase = static_cast<OBS::TutorialPhase>(phase);
    state->objectives.push_back(obj);
    return true;
}

bool OnboardingSystem::completeObjective(const std::string& entity_id,
    const std::string& objective_id, float completed_at) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& o : state->objectives) {
        if (o.objective_id == objective_id) {
            if (o.completed) return false;  // already completed
            o.completed = true;
            o.completed_at = completed_at;
            state->objectives_completed++;
            return true;
        }
    }
    return false;
}

bool OnboardingSystem::isObjectiveComplete(const std::string& entity_id,
    const std::string& objective_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& o : state->objectives) {
        if (o.objective_id == objective_id) return o.completed;
    }
    return false;
}

int OnboardingSystem::getObjectiveCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->objectives.size()) : 0;
}

int OnboardingSystem::getCompletedObjectiveCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->objectives_completed : 0;
}

bool OnboardingSystem::addHint(const std::string& entity_id,
    const std::string& hint_id, const std::string& text) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->hints.size()) >= state->max_hints) return false;
    for (const auto& h : state->hints) {
        if (h.hint_id == hint_id) return false;
    }
    OBS::HintEntry hint;
    hint.hint_id = hint_id;
    hint.text = text;
    state->hints.push_back(hint);
    return true;
}

bool OnboardingSystem::showHint(const std::string& entity_id,
    const std::string& hint_id, float shown_at) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& h : state->hints) {
        if (h.hint_id == hint_id) {
            if (h.shown) return false;  // already shown
            h.shown = true;
            h.shown_at = shown_at;
            state->hints_shown++;
            return true;
        }
    }
    return false;
}

bool OnboardingSystem::isHintShown(const std::string& entity_id,
    const std::string& hint_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& h : state->hints) {
        if (h.hint_id == hint_id) return h.shown;
    }
    return false;
}

int OnboardingSystem::getHintCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->hints.size()) : 0;
}

int OnboardingSystem::getShownHintCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->hints_shown : 0;
}

bool OnboardingSystem::isTutorialComplete(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->tutorial_complete : false;
}

bool OnboardingSystem::isTutorialSkipped(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->tutorial_skipped : false;
}

float OnboardingSystem::getCompletionTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->completion_time : 0.0f;
}

} // namespace systems
} // namespace atlas
