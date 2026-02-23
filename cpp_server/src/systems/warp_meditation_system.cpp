#include "systems/warp_meditation_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

WarpMeditationSystem::WarpMeditationSystem(ecs::World* world)
    : System(world) {
}

void WarpMeditationSystem::update(float delta_time) {
    // Update meditation layers
    auto med_entities = world_->getEntities<components::WarpMeditationLayer>();
    for (auto* entity : med_entities) {
        auto* med = entity->getComponent<components::WarpMeditationLayer>();
        auto* warp = entity->getComponent<components::WarpState>();
        if (!med || !warp) continue;

        if (warp->phase == components::WarpState::WarpPhase::Cruise) {
            med->warp_cruise_time += delta_time;
            if (med->warp_cruise_time >= med->activation_delay && !med->active) {
                med->active = true;
            }
            if (med->active) {
                med->volume += delta_time / med->fade_duration;
                med->volume = std::min(med->volume, 1.0f);
            }
        } else {
            // Fade out when not in Cruise
            med->volume -= delta_time / med->fade_duration;
            med->volume = std::max(med->volume, 0.0f);
            if (med->volume <= 0.0f) {
                med->active = false;
            }
            med->warp_cruise_time = 0.0f;
        }
    }

    // Update audio progression
    auto prog_entities = world_->getEntities<components::WarpAudioProgression>();
    for (auto* entity : prog_entities) {
        auto* prog = entity->getComponent<components::WarpAudioProgression>();
        auto* warp = entity->getComponent<components::WarpState>();
        if (!prog || !warp) continue;

        using WP = components::WarpState::WarpPhase;
        using Phase = components::WarpAudioProgression::Phase;

        if (warp->phase == WP::Cruise || warp->phase == WP::Entry || warp->phase == WP::Align) {
            prog->phase_timer += delta_time;

            // Progress through phases
            float elapsed = prog->phase_timer;
            if (elapsed < prog->tension_duration) {
                prog->current_phase = Phase::Tension;
                prog->blend_factor = elapsed / prog->tension_duration;
            } else if (elapsed < prog->tension_duration + prog->stabilize_duration) {
                prog->current_phase = Phase::Stabilize;
                prog->blend_factor = (elapsed - prog->tension_duration) / prog->stabilize_duration;
            } else if (elapsed < prog->tension_duration + prog->stabilize_duration + prog->bloom_duration) {
                prog->current_phase = Phase::Bloom;
                prog->blend_factor = (elapsed - prog->tension_duration - prog->stabilize_duration) / prog->bloom_duration;
            } else {
                prog->current_phase = Phase::Meditative;
                prog->blend_factor = 1.0f;
            }
        } else if (warp->phase == WP::None || warp->phase == WP::Exit) {
            prog->current_phase = Phase::Tension;
            prog->phase_timer = 0.0f;
            prog->blend_factor = 0.0f;
        }
    }
}

float WarpMeditationSystem::getMeditationVolume(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* med = entity->getComponent<components::WarpMeditationLayer>();
    if (!med) return 0.0f;
    return med->volume;
}

bool WarpMeditationSystem::isMeditationActive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* med = entity->getComponent<components::WarpMeditationLayer>();
    if (!med) return false;
    return med->active;
}

components::WarpAudioProgression::Phase
WarpMeditationSystem::getAudioPhase(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::WarpAudioProgression::Phase::Tension;
    auto* prog = entity->getComponent<components::WarpAudioProgression>();
    if (!prog) return components::WarpAudioProgression::Phase::Tension;
    return prog->current_phase;
}

float WarpMeditationSystem::getAudioProgression(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* prog = entity->getComponent<components::WarpAudioProgression>();
    if (!prog) return 0.0f;
    return prog->computeOverallProgression();
}

} // namespace systems
} // namespace atlas
