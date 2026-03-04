#include "systems/crew_activity_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

CrewActivitySystem::CrewActivitySystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CrewActivitySystem::updateComponent(ecs::Entity& /*entity*/, components::CrewActivity& crew, float delta_time) {
    crew.activity_timer += delta_time;

    // Accumulate needs over time
    crew.fatigue += 0.002f * delta_time;
    crew.hunger += 0.003f * delta_time;
    if (crew.fatigue > 1.0f) crew.fatigue = 1.0f;
    if (crew.hunger > 1.0f) crew.hunger = 1.0f;

    // Priority-based activity transitions
    // 1. Ship damage overrides everything — repair
    if (crew.ship_damaged && crew.current_activity != components::CrewActivity::Activity::Repairing) {
        crew.current_activity = components::CrewActivity::Activity::Repairing;
        crew.activity_timer = 0.0f;
        return;
    }

    // 2. High hunger — go eat
    if (crew.hunger >= 0.8f && crew.current_activity != components::CrewActivity::Activity::Eating) {
        crew.current_activity = components::CrewActivity::Activity::Eating;
        crew.activity_timer = 0.0f;
        return;
    }

    // 3. High fatigue — go rest
    if (crew.fatigue >= 0.8f && crew.current_activity != components::CrewActivity::Activity::Resting) {
        crew.current_activity = components::CrewActivity::Activity::Resting;
        crew.activity_timer = 0.0f;
        return;
    }

    // Activity completion
    if (crew.activity_timer >= crew.activity_duration) {
        crew.activity_timer = 0.0f;

        switch (crew.current_activity) {
            case components::CrewActivity::Activity::Resting:
                crew.fatigue = std::max(0.0f, crew.fatigue - 0.5f);
                crew.current_activity = components::CrewActivity::Activity::Working;
                break;
            case components::CrewActivity::Activity::Eating:
                crew.hunger = std::max(0.0f, crew.hunger - 0.6f);
                crew.current_activity = components::CrewActivity::Activity::Working;
                break;
            case components::CrewActivity::Activity::Repairing:
                if (!crew.ship_damaged) {
                    crew.current_activity = components::CrewActivity::Activity::Working;
                }
                break;
            case components::CrewActivity::Activity::Idle:
                if (!crew.assigned_room_id.empty()) {
                    crew.current_activity = components::CrewActivity::Activity::Walking;
                }
                break;
            case components::CrewActivity::Activity::Walking:
                crew.current_activity = components::CrewActivity::Activity::Manning;
                crew.station_manned = true;
                break;
            case components::CrewActivity::Activity::Manning:
            case components::CrewActivity::Activity::Working:
            default:
                crew.current_activity = components::CrewActivity::Activity::Working;
                break;
        }
    }
}

void CrewActivitySystem::assignRoom(const std::string& crew_entity_id, const std::string& room_id) {
    auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return;
    crew->assigned_room_id = room_id;
}

void CrewActivitySystem::setShipDamaged(const std::string& crew_entity_id, bool damaged) {
    auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return;
    crew->ship_damaged = damaged;
}

std::string CrewActivitySystem::getActivity(const std::string& crew_entity_id) const {
    const auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return "Unknown";
    return components::CrewActivity::activityToString(crew->current_activity);
}

std::string CrewActivitySystem::getAssignedRoom(const std::string& crew_entity_id) const {
    const auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return "";
    return crew->assigned_room_id;
}

float CrewActivitySystem::getFatigue(const std::string& crew_entity_id) const {
    const auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return 0.0f;
    return crew->fatigue;
}

float CrewActivitySystem::getHunger(const std::string& crew_entity_id) const {
    const auto* crew = getComponentFor(crew_entity_id);
    if (!crew) return 0.0f;
    return crew->hunger;
}

std::vector<std::string> CrewActivitySystem::getCrewInActivity(
    components::CrewActivity::Activity activity) const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::CrewActivity>();
    for (auto* entity : entities) {
        auto* crew = entity->getComponent<components::CrewActivity>();
        if (crew && crew->current_activity == activity) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
