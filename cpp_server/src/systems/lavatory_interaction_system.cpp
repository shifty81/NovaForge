#include "systems/lavatory_interaction_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

LavatoryInteractionSystem::LavatoryInteractionSystem(ecs::World* world)
    : System(world) {
}

void LavatoryInteractionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::LavatoryInteraction>();
    for (auto* entity : entities) {
        auto* lav = entity->getComponent<components::LavatoryInteraction>();
        if (!lav || lav->phase == 0) continue; // Idle, skip

        using Phase = components::LavatoryInteraction::InteractionPhase;
        auto currentPhase = static_cast<Phase>(lav->phase);

        if (currentPhase == Phase::UsingFacility) {
            // Special: use_timer counts up to use_duration
            lav->use_timer += delta_time;
            float safe_duration = (lav->use_duration > 0.0f) ? lav->use_duration : 1.0f;
            lav->phase_progress = lav->use_timer / safe_duration;
            if (lav->use_timer >= lav->use_duration) {
                lav->phase_progress = 1.0f;
                lav->use_timer = lav->use_duration;
                lav->phase = static_cast<int>(Phase::TransitionToFirstPerson);
                lav->phase_progress = 0.0f;
                lav->in_third_person = false;
            }
        } else if (currentPhase != Phase::Idle && currentPhase != Phase::Complete) {
            lav->phase_progress += delta_time / std::max(lav->phase_duration, 0.001f);
            if (lav->phase_progress >= 1.0f) {
                lav->phase_progress = 1.0f;
                // Advance to next phase
                int next = lav->phase + 1;
                if (next > static_cast<int>(Phase::Complete)) {
                    next = static_cast<int>(Phase::Complete);
                }
                lav->phase = next;
                lav->phase_progress = 0.0f;

                // Apply side effects on phase entry
                auto newPhase = static_cast<Phase>(lav->phase);
                switch (newPhase) {
                    case Phase::DoorOpening:
                        lav->door_open = true;
                        lav->audio_playing = true;
                        break;
                    case Phase::TransitionToThirdPerson:
                        lav->in_third_person = true;
                        break;
                    case Phase::UsingFacility:
                        lav->use_timer = 0.0f;
                        break;
                    case Phase::TransitionToFirstPerson:
                        lav->in_third_person = false;
                        break;
                    case Phase::DoorClosing:
                        lav->door_open = false;
                        break;
                    case Phase::Complete:
                        lav->occupied = false;
                        lav->audio_playing = false;
                        lav->user_id.clear();
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

bool LavatoryInteractionSystem::createLavatory(const std::string& entity_id,
                                                const std::string& room_id,
                                                float use_duration,
                                                float hygiene_bonus) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::LavatoryInteraction>()) return false;

    auto comp = std::make_unique<components::LavatoryInteraction>();
    comp->lavatory_id = entity_id;
    comp->room_id = room_id;
    comp->use_duration = use_duration;
    comp->hygiene_bonus = hygiene_bonus;
    entity->addComponent(std::move(comp));
    return true;
}

bool LavatoryInteractionSystem::beginInteraction(const std::string& entity_id,
                                                  const std::string& user_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    if (lav->occupied) return false;
    if (lav->phase != 0) return false;

    lav->user_id = user_id;
    lav->occupied = true;
    lav->phase = static_cast<int>(components::LavatoryInteraction::InteractionPhase::Approaching);
    lav->phase_progress = 0.0f;
    return true;
}

bool LavatoryInteractionSystem::cancelInteraction(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    if (lav->phase == 0) return false;

    lav->phase = 0;
    lav->phase_progress = 0.0f;
    lav->occupied = false;
    lav->door_open = false;
    lav->in_third_person = false;
    lav->audio_playing = false;
    lav->use_timer = 0.0f;
    lav->user_id.clear();
    return true;
}

int LavatoryInteractionSystem::getPhase(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return 0;
    return lav->phase;
}

float LavatoryInteractionSystem::getPhaseProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return 0.0f;
    return lav->phase_progress;
}

bool LavatoryInteractionSystem::isOccupied(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    return lav->occupied;
}

bool LavatoryInteractionSystem::isDoorOpen(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    return lav->door_open;
}

bool LavatoryInteractionSystem::isInThirdPerson(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    return lav->in_third_person;
}

bool LavatoryInteractionSystem::isAudioPlaying(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return false;
    return lav->audio_playing;
}

float LavatoryInteractionSystem::getHygieneBonus(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* lav = entity->getComponent<components::LavatoryInteraction>();
    if (!lav) return 0.0f;
    return lav->hygiene_bonus;
}

std::string LavatoryInteractionSystem::phaseName(int phase) {
    using Phase = components::LavatoryInteraction::InteractionPhase;
    switch (static_cast<Phase>(phase)) {
        case Phase::Idle: return "idle";
        case Phase::Approaching: return "approaching";
        case Phase::DoorOpening: return "door_opening";
        case Phase::TransitionToThirdPerson: return "transition_to_third_person";
        case Phase::UsingFacility: return "using_facility";
        case Phase::TransitionToFirstPerson: return "transition_to_first_person";
        case Phase::DoorClosing: return "door_closing";
        case Phase::Complete: return "complete";
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
