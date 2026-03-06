#include "systems/objective_tracker_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <cmath>
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using OT = components::ObjectiveTrackerState;
}

ObjectiveTrackerSystem::ObjectiveTrackerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ObjectiveTrackerSystem::updateComponent(ecs::Entity& entity,
    components::ObjectiveTrackerState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool ObjectiveTrackerSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ObjectiveTrackerState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ObjectiveTrackerSystem::addObjective(const std::string& entity_id,
    const std::string& objective_id, const std::string& description,
    const std::string& category, float target_x, float target_y) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->objectives.size()) >= state->max_objectives) return false;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) return false;
    }
    OT::TrackedObjective obj;
    obj.objective_id = objective_id;
    obj.description = description;
    obj.category = category;
    obj.target_x = target_x;
    obj.target_y = target_y;
    state->objectives.push_back(obj);

    // Auto-set active if first objective
    if (state->active_objective_id.empty()) {
        state->active_objective_id = objective_id;
    }
    return true;
}

bool ObjectiveTrackerSystem::removeObjective(const std::string& entity_id,
    const std::string& objective_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->objectives.begin(), state->objectives.end(),
        [&](const OT::TrackedObjective& o) { return o.objective_id == objective_id; });
    if (it == state->objectives.end()) return false;
    state->objectives.erase(it);
    if (state->active_objective_id == objective_id) {
        state->active_objective_id.clear();
    }
    return true;
}

int ObjectiveTrackerSystem::getObjectiveCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->objectives.size()) : 0;
}

bool ObjectiveTrackerSystem::hasObjective(const std::string& entity_id,
    const std::string& objective_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) return true;
    }
    return false;
}

bool ObjectiveTrackerSystem::updateProgress(const std::string& entity_id,
    const std::string& objective_id, float progress) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) {
            if (obj.completed) return false;
            obj.progress = std::max(0.0f, std::min(1.0f, progress));
            return true;
        }
    }
    return false;
}

float ObjectiveTrackerSystem::getProgress(const std::string& entity_id,
    const std::string& objective_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) return obj.progress;
    }
    return 0.0f;
}

bool ObjectiveTrackerSystem::completeObjective(const std::string& entity_id,
    const std::string& objective_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) {
            if (obj.completed) return false;
            obj.completed = true;
            obj.progress = 1.0f;
            obj.completed_at = timestamp;
            state->completed_count++;
            return true;
        }
    }
    return false;
}

bool ObjectiveTrackerSystem::isObjectiveComplete(const std::string& entity_id,
    const std::string& objective_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) return obj.completed;
    }
    return false;
}

bool ObjectiveTrackerSystem::setActiveObjective(const std::string& entity_id,
    const std::string& objective_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) {
            state->active_objective_id = objective_id;
            return true;
        }
    }
    return false;
}

std::string ObjectiveTrackerSystem::getActiveObjectiveId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->active_objective_id : "";
}

bool ObjectiveTrackerSystem::updatePlayerPosition(const std::string& entity_id,
    float px, float py) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->player_x = px;
    state->player_y = py;
    return true;
}

float ObjectiveTrackerSystem::getDistanceToObjective(const std::string& entity_id,
    const std::string& objective_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return -1.0f;
    for (const auto& obj : state->objectives) {
        if (obj.objective_id == objective_id) {
            float dx = obj.target_x - state->player_x;
            float dy = obj.target_y - state->player_y;
            return std::sqrt(dx * dx + dy * dy);
        }
    }
    return -1.0f;
}

float ObjectiveTrackerSystem::getDistanceToActive(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->active_objective_id.empty()) return -1.0f;
    return getDistanceToObjective(entity_id, state->active_objective_id);
}

int ObjectiveTrackerSystem::getCompletedCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->completed_count : 0;
}

float ObjectiveTrackerSystem::getOverallCompletion(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->objectives.empty()) return 0.0f;
    float total = 0.0f;
    for (const auto& obj : state->objectives) {
        total += obj.progress;
    }
    return total / static_cast<float>(state->objectives.size());
}

} // namespace systems
} // namespace atlas
