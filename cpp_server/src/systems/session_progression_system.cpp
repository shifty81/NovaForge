#include "systems/session_progression_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SP = components::SessionProgression;
}

SessionProgressionSystem::SessionProgressionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SessionProgressionSystem::updateComponent(ecs::Entity& entity,
    components::SessionProgression& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool SessionProgressionSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SessionProgression>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SessionProgressionSystem::recordMilestone(const std::string& entity_id,
    const std::string& milestone_id, const std::string& description, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->milestones.size()) >= state->max_milestones) return false;
    for (const auto& m : state->milestones) {
        if (m.milestone_id == milestone_id) return false;
    }
    SP::Milestone ms;
    ms.milestone_id = milestone_id;
    ms.description = description;
    ms.timestamp = timestamp;
    ms.reached = true;
    state->milestones.push_back(ms);
    return true;
}

bool SessionProgressionSystem::isMilestoneReached(const std::string& entity_id,
    const std::string& milestone_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& m : state->milestones) {
        if (m.milestone_id == milestone_id) return m.reached;
    }
    return false;
}

int SessionProgressionSystem::getMilestoneCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->milestones.size()) : 0;
}

bool SessionProgressionSystem::addStatistic(const std::string& entity_id,
    const std::string& stat_key, double value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& s : state->statistics) {
        if (s.stat_key == stat_key) {
            s.value += value;
            return true;
        }
    }
    SP::Statistic stat;
    stat.stat_key = stat_key;
    stat.value = value;
    state->statistics.push_back(stat);
    return true;
}

double SessionProgressionSystem::getStatistic(const std::string& entity_id,
    const std::string& stat_key) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& s : state->statistics) {
        if (s.stat_key == stat_key) return s.value;
    }
    return 0.0;
}

int SessionProgressionSystem::getStatisticCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->statistics.size()) : 0;
}

bool SessionProgressionSystem::logActivity(const std::string& entity_id,
    const std::string& activity_type, const std::string& detail, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->activities.size()) >= state->max_activities) {
        state->activities.erase(state->activities.begin());
    }
    SP::ActivityEntry entry;
    entry.activity_type = activity_type;
    entry.detail = detail;
    entry.timestamp = timestamp;
    state->activities.push_back(entry);
    return true;
}

int SessionProgressionSystem::getActivityCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->activities.size()) : 0;
}

std::string SessionProgressionSystem::getLastActivityType(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->activities.empty()) return "";
    return state->activities.back().activity_type;
}

float SessionProgressionSystem::getSessionDuration(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->elapsed_time : 0.0f;
}

bool SessionProgressionSystem::finalizeSession(const std::string& entity_id, float end_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->session_finalized) return false;
    state->session_finalized = true;
    state->session_end_time = end_time;
    return true;
}

bool SessionProgressionSystem::isSessionFinalized(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->session_finalized : false;
}

} // namespace systems
} // namespace atlas
