#include "systems/emotional_arc_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

EmotionalArcSystem::EmotionalArcSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void EmotionalArcSystem::updateComponent(ecs::Entity& entity, components::EmotionalState& comp, float delta_time) {
    // Fatigue increases by 0.01 per minute of active play
    comp.fatigue += 0.01f * (delta_time / 60.0f);
    comp.fatigue = std::clamp(comp.fatigue, 0.0f, 100.0f);

    // Confidence and hope drift slowly toward neutral (50)
    float drift_rate = 0.05f * (delta_time / 60.0f);
    if (comp.confidence > 50.0f) {
        comp.confidence = std::max(50.0f, comp.confidence - drift_rate);
    } else if (comp.confidence < 50.0f) {
        comp.confidence = std::min(50.0f, comp.confidence + drift_rate);
    }

    if (comp.hope > 50.0f) {
        comp.hope = std::max(50.0f, comp.hope - drift_rate);
    } else if (comp.hope < 50.0f) {
        comp.hope = std::min(50.0f, comp.hope + drift_rate);
    }
}

void EmotionalArcSystem::onCombatVictory(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->confidence = std::clamp(state->confidence + 5.0f, 0.0f, 100.0f);
    state->hope = std::clamp(state->hope + 3.0f, 0.0f, 100.0f);
    state->fatigue = std::clamp(state->fatigue + 2.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onCombatDefeat(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->confidence = std::clamp(state->confidence - 8.0f, 0.0f, 100.0f);
    state->hope = std::clamp(state->hope - 5.0f, 0.0f, 100.0f);
    state->fatigue = std::clamp(state->fatigue + 5.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onRest(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->fatigue = std::max(0.0f, state->fatigue - 10.0f);
}

void EmotionalArcSystem::onPlayerTrust(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->trust_in_player = std::clamp(state->trust_in_player + 5.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onPlayerBetray(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->trust_in_player = std::clamp(state->trust_in_player - 15.0f, 0.0f, 100.0f);
}

float EmotionalArcSystem::getConfidence(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 50.0f;

    return state->confidence;
}

float EmotionalArcSystem::getTrust(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 50.0f;

    return state->trust_in_player;
}

float EmotionalArcSystem::getFatigue(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;

    return state->fatigue;
}

float EmotionalArcSystem::getHope(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 50.0f;

    return state->hope;
}

} // namespace systems
} // namespace atlas
