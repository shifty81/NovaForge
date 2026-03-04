#include "systems/eva_airlock_system.h"
#include "ecs/world.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

EVAAirlockSystem::EVAAirlockSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void EVAAirlockSystem::updateComponent(ecs::Entity& /*entity*/, components::EVAAirlockState& state, float delta_time) {
    using P = components::EVAAirlockState::Phase;
    int phase = state.phase;

    // Idle or EVAActive or Complete — no progression needed
    if (phase == static_cast<int>(P::Idle) ||
        phase == static_cast<int>(P::EVAActive) ||
        phase == static_cast<int>(P::Complete)) {
        return;
    }

    // Handle abort
    if (state.abort_requested) {
        // Can abort up to and including Depressurize (before outer door opens)
        if (phase <= static_cast<int>(P::Depressurize)) {
            state.phase = static_cast<int>(P::Idle);
            state.phase_progress = 0.0f;
            state.chamber_pressure = 1.0f;
            state.inner_door_open = false;
            state.outer_door_open = false;
            state.player_id.clear();
            state.abort_requested = false;
            return;
        }
        state.abort_requested = false;  // Can't abort after outer door open
    }

    // Advance phase progress
    float duration = std::max(0.01f, state.phase_duration);
    state.phase_progress += delta_time / duration;

    if (state.phase_progress < 1.0f) {
        // Update pressure interpolation during depressurize/repressurize
        if (phase == static_cast<int>(P::Depressurize)) {
            state.chamber_pressure = 1.0f - state.phase_progress;
        } else if (phase == static_cast<int>(P::Repressurize)) {
            state.chamber_pressure = state.phase_progress;
        }
        return;
    }

    // Phase complete — advance to next
    state.phase_progress = 0.0f;

    if (phase == static_cast<int>(P::EnterChamber)) {
        state.inner_door_open = false;
        state.phase = static_cast<int>(P::InnerSeal);
    }
    else if (phase == static_cast<int>(P::InnerSeal)) {
        state.phase = static_cast<int>(P::Depressurize);
    }
    else if (phase == static_cast<int>(P::Depressurize)) {
        state.chamber_pressure = 0.0f;
        state.outer_door_open = true;
        state.phase = static_cast<int>(P::OuterOpen);
    }
    else if (phase == static_cast<int>(P::OuterOpen)) {
        state.phase = static_cast<int>(P::EVAActive);
    }
    // Re-entry phases
    else if (phase == static_cast<int>(P::OuterSeal)) {
        state.outer_door_open = false;
        state.phase = static_cast<int>(P::Repressurize);
    }
    else if (phase == static_cast<int>(P::Repressurize)) {
        state.chamber_pressure = 1.0f;
        state.inner_door_open = true;
        state.phase = static_cast<int>(P::InnerOpen);
    }
    else if (phase == static_cast<int>(P::InnerOpen)) {
        state.phase = static_cast<int>(P::Complete);
        state.inner_door_open = false;
    }
}

bool EVAAirlockSystem::createAirlock(const std::string& airlock_id,
                                      const std::string& ship_id,
                                      float phase_duration) {
    if (world_->getEntity(airlock_id)) return false;

    auto* entity = world_->createEntity(airlock_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::EVAAirlockState>();
    comp->airlock_id = airlock_id;
    comp->ship_id = ship_id;
    comp->phase_duration = std::max(0.1f, phase_duration);
    entity->addComponent(std::move(comp));
    return true;
}

bool EVAAirlockSystem::beginEVA(const std::string& airlock_id,
                                 const std::string& player_id,
                                 float suit_oxygen) {
    auto* st = getComponentFor(airlock_id);
    if (!st) return false;

    using P = components::EVAAirlockState::Phase;

    // Must be idle
    if (st->phase != static_cast<int>(P::Idle)) return false;

    // Check suit oxygen
    if (suit_oxygen < st->min_suit_oxygen) {
        st->suit_check_passed = false;
        return false;
    }

    st->suit_check_passed = true;
    st->player_id = player_id;
    st->inner_door_open = true;
    st->phase = static_cast<int>(P::EnterChamber);
    st->phase_progress = 0.0f;
    st->abort_requested = false;
    return true;
}

bool EVAAirlockSystem::beginReentry(const std::string& airlock_id,
                                     const std::string& player_id) {
    auto* st = getComponentFor(airlock_id);
    if (!st) return false;

    using P = components::EVAAirlockState::Phase;

    // Must be in EVA
    if (st->phase != static_cast<int>(P::EVAActive)) return false;
    // Must be the same player
    if (st->player_id != player_id) return false;

    st->phase = static_cast<int>(P::OuterSeal);
    st->phase_progress = 0.0f;
    return true;
}

bool EVAAirlockSystem::abortSequence(const std::string& airlock_id) {
    auto* st = getComponentFor(airlock_id);
    if (!st) return false;

    using P = components::EVAAirlockState::Phase;

    // Can only abort during pre-EVA phases
    if (st->phase == static_cast<int>(P::Idle) ||
        st->phase >= static_cast<int>(P::EVAActive)) {
        return false;
    }

    st->abort_requested = true;
    return true;
}

int EVAAirlockSystem::getPhase(const std::string& airlock_id) const {
    const auto* st = getComponentFor(airlock_id);
    return st ? st->phase : -1;
}

float EVAAirlockSystem::getPhaseProgress(const std::string& airlock_id) const {
    const auto* st = getComponentFor(airlock_id);
    return st ? st->phase_progress : 0.0f;
}

float EVAAirlockSystem::getChamberPressure(const std::string& airlock_id) const {
    const auto* st = getComponentFor(airlock_id);
    return st ? st->chamber_pressure : 0.0f;
}

bool EVAAirlockSystem::isInEVA(const std::string& airlock_id) const {
    const auto* st = getComponentFor(airlock_id);
    if (!st) return false;
    return st->phase == static_cast<int>(components::EVAAirlockState::Phase::EVAActive);
}

std::string EVAAirlockSystem::phaseName(int phase) {
    using P = components::EVAAirlockState::Phase;
    switch (static_cast<P>(phase)) {
        case P::Idle:           return "Idle";
        case P::EnterChamber:   return "EnterChamber";
        case P::InnerSeal:      return "InnerSeal";
        case P::Depressurize:   return "Depressurize";
        case P::OuterOpen:      return "OuterOpen";
        case P::EVAActive:      return "EVAActive";
        case P::OuterSeal:      return "OuterSeal";
        case P::Repressurize:   return "Repressurize";
        case P::InnerOpen:      return "InnerOpen";
        case P::Complete:       return "Complete";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
