#include "systems/space_planet_transition_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

SpacePlanetTransitionSystem::SpacePlanetTransitionSystem(ecs::World* world)
    : System(world) {
}

void SpacePlanetTransitionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::SpacePlanetTransition>();
    for (auto* entity : entities) {
        auto* tr = entity->getComponent<components::SpacePlanetTransition>();
        if (!tr) continue;

        // Only process entities in transition
        if (!tr->isInTransition()) continue;

        // Phase durations (seconds)
        float phase_duration = 10.0f;

        tr->current_phase_time += delta_time;
        tr->total_transition_time += delta_time;
        tr->progress += delta_time / phase_duration;
        tr->progress = std::min(tr->progress, 1.0f);

        // Update altitude and heat based on phase
        using TS = components::SpacePlanetTransition::TransitionState;

        switch (tr->transition_state) {
            case TS::OrbitEntry:
                tr->altitude -= delta_time * 50.0f;
                tr->altitude = std::max(tr->altitude, 500.0f);
                break;
            case TS::AtmosphereEntry:
                tr->altitude -= delta_time * 80.0f;
                tr->altitude = std::max(tr->altitude, 200.0f);
                if (tr->has_atmosphere) {
                    tr->heat_buildup += delta_time * 10.0f * tr->atmosphere_density;
                }
                break;
            case TS::DescentPhase:
                tr->altitude -= delta_time * 60.0f;
                tr->altitude = std::max(tr->altitude, 50.0f);
                // Cooling during descent
                tr->heat_buildup = std::max(0.0f, tr->heat_buildup - delta_time * 2.0f);
                break;
            case TS::LandingApproach:
                tr->altitude -= delta_time * 20.0f;
                tr->altitude = std::max(tr->altitude, 0.0f);
                tr->heat_buildup = std::max(0.0f, tr->heat_buildup - delta_time * 3.0f);
                break;
            case TS::LaunchSequence:
                tr->altitude += delta_time * 30.0f;
                break;
            case TS::AtmosphereExit:
                tr->altitude += delta_time * 80.0f;
                if (tr->has_atmosphere) {
                    tr->heat_buildup += delta_time * 8.0f * tr->atmosphere_density;
                }
                break;
            case TS::OrbitExit:
                tr->altitude += delta_time * 50.0f;
                tr->heat_buildup = std::max(0.0f, tr->heat_buildup - delta_time * 2.0f);
                break;
            default:
                break;
        }

        // Hull stress from excessive heat
        if (tr->heat_buildup > tr->max_heat_tolerance) {
            tr->hull_stress += (tr->heat_buildup - tr->max_heat_tolerance) * delta_time * 0.01f;
        }

        // Update gravity based on altitude
        tr->gravity_factor = components::SpacePlanetTransition::getGravityForAltitude(tr->altitude);

        // Auto-advance to next phase when progress reaches 1.0
        if (tr->progress >= 1.0f) {
            tr->progress = 0.0f;
            tr->current_phase_time = 0.0f;

            switch (tr->transition_state) {
                case TS::OrbitEntry:
                    tr->transition_state = TS::AtmosphereEntry;
                    break;
                case TS::AtmosphereEntry:
                    tr->transition_state = TS::DescentPhase;
                    break;
                case TS::DescentPhase:
                    tr->transition_state = TS::LandingApproach;
                    break;
                case TS::LandingApproach:
                    tr->transition_state = TS::Landed;
                    tr->altitude = 0.0f;
                    tr->speed = 0.0f;
                    break;
                case TS::LaunchSequence:
                    tr->transition_state = TS::AtmosphereExit;
                    break;
                case TS::AtmosphereExit:
                    tr->transition_state = TS::OrbitExit;
                    break;
                case TS::OrbitExit:
                    tr->transition_state = TS::InSpace;
                    tr->altitude = 1000.0f;
                    break;
                default:
                    break;
            }
        }
    }
}

bool SpacePlanetTransitionSystem::initializeTransition(const std::string& entity_id,
                                                        const std::string& planet_id,
                                                        bool has_atmosphere) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::SpacePlanetTransition>();
    if (existing) return false;

    auto comp = std::make_unique<components::SpacePlanetTransition>();
    comp->entity_id = entity_id;
    comp->planet_id = planet_id;
    comp->has_atmosphere = has_atmosphere;
    comp->atmosphere_density = has_atmosphere ? 0.7f : 0.0f;
    comp->altitude = 1000.0f;
    entity->addComponent(std::move(comp));
    return true;
}

bool SpacePlanetTransitionSystem::beginDescent(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return false;

    if (tr->transition_state != components::SpacePlanetTransition::TransitionState::InSpace)
        return false;

    tr->transition_state = components::SpacePlanetTransition::TransitionState::OrbitEntry;
    tr->progress = 0.0f;
    tr->current_phase_time = 0.0f;
    return true;
}

bool SpacePlanetTransitionSystem::beginLaunch(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return false;

    if (tr->transition_state != components::SpacePlanetTransition::TransitionState::Landed)
        return false;

    tr->transition_state = components::SpacePlanetTransition::TransitionState::LaunchSequence;
    tr->progress = 0.0f;
    tr->current_phase_time = 0.0f;
    return true;
}

bool SpacePlanetTransitionSystem::abortTransition(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return false;

    using TS = components::SpacePlanetTransition::TransitionState;

    // Can't abort when already Landed or InSpace
    if (tr->transition_state == TS::Landed || tr->transition_state == TS::InSpace)
        return false;

    // Abort returns to InSpace
    tr->transition_state = TS::InSpace;
    tr->progress = 0.0f;
    tr->current_phase_time = 0.0f;
    tr->altitude = 1000.0f;
    return true;
}

bool SpacePlanetTransitionSystem::setAutopilot(const std::string& entity_id, bool enabled) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return false;

    tr->is_autopilot = enabled;
    return true;
}

bool SpacePlanetTransitionSystem::setLandingTarget(const std::string& entity_id, float x, float y) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return false;

    tr->target_landing_x = x;
    tr->target_landing_y = y;
    return true;
}

float SpacePlanetTransitionSystem::getAltitude(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return 0.0f;

    return tr->altitude;
}

float SpacePlanetTransitionSystem::getHeatLevel(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return 0.0f;

    return tr->heat_buildup;
}

std::string SpacePlanetTransitionSystem::getTransitionState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    if (!tr) return "unknown";

    return components::SpacePlanetTransition::stateToString(tr->transition_state);
}

} // namespace systems
} // namespace atlas
