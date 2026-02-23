#include "systems/captain_departure_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

CaptainDepartureSystem::CaptainDepartureSystem(ecs::World* world)
    : System(world) {
}

void CaptainDepartureSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::CaptainDepartureState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::CaptainDepartureState>();
        if (!state) continue;

        using DP = components::CaptainDepartureState::DeparturePhase;

        if (state->phase == DP::None && state->disagreement_score >= state->grumble_threshold) {
            state->phase = DP::Grumbling;
        }
        if (state->phase == DP::Grumbling && state->disagreement_score >= state->request_threshold) {
            state->phase = DP::FormalRequest;
            state->departure_timer = state->departure_delay;
        }
        if (state->phase == DP::FormalRequest) {
            if (state->player_acknowledged) {
                state->phase = DP::Departing;
            } else {
                state->departure_timer -= delta_time;
                if (state->departure_timer <= 0.0f) {
                    state->phase = DP::Departing;
                }
            }
        }
    }
}

void CaptainDepartureSystem::addDisagreement(const std::string& entity_id, float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* state = entity->getComponent<components::CaptainDepartureState>();
    if (!state) return;
    state->disagreement_score += amount;
}

components::CaptainDepartureState::DeparturePhase
CaptainDepartureSystem::getDeparturePhase(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::CaptainDepartureState::DeparturePhase::None;
    auto* state = entity->getComponent<components::CaptainDepartureState>();
    if (!state) return components::CaptainDepartureState::DeparturePhase::None;
    return state->phase;
}

void CaptainDepartureSystem::acknowledgeRequest(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* state = entity->getComponent<components::CaptainDepartureState>();
    if (!state) return;
    state->player_acknowledged = true;
}

std::vector<std::string> CaptainDepartureSystem::getDepartingCaptains() const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::CaptainDepartureState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::CaptainDepartureState>();
        if (state && state->phase == components::CaptainDepartureState::DeparturePhase::Departing) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
