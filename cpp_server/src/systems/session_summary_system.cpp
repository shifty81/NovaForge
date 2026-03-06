#include "systems/session_summary_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SS = components::SessionSummaryState;
}

SessionSummarySystem::SessionSummarySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SessionSummarySystem::updateComponent(ecs::Entity& entity,
    components::SessionSummaryState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool SessionSummarySystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SessionSummaryState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SessionSummarySystem::recordStat(const std::string& entity_id,
    const std::string& stat_key, double value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->stats.size()) >= state->max_stats) {
        // Check if key already exists — update it
        for (auto& s : state->stats) {
            if (s.stat_key == stat_key) {
                s.value += value;
                return true;
            }
        }
        return false;
    }
    for (auto& s : state->stats) {
        if (s.stat_key == stat_key) {
            s.value += value;
            return true;
        }
    }
    SS::SummaryStat st;
    st.stat_key = stat_key;
    st.value = value;
    state->stats.push_back(st);
    return true;
}

double SessionSummarySystem::getStat(const std::string& entity_id,
    const std::string& stat_key) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& s : state->stats) {
        if (s.stat_key == stat_key) return s.value;
    }
    return 0.0;
}

int SessionSummarySystem::getStatCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->stats.size()) : 0;
}

bool SessionSummarySystem::addCategoryStat(const std::string& entity_id,
    const std::string& category, const std::string& stat_key, double value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Find or create category
    for (auto& cat : state->categories) {
        if (cat.category == category) {
            for (auto& s : cat.entries) {
                if (s.stat_key == stat_key) {
                    s.value += value;
                    return true;
                }
            }
            if (static_cast<int>(cat.entries.size()) >= state->max_category_entries) return false;
            SS::SummaryStat st;
            st.stat_key = stat_key;
            st.value = value;
            cat.entries.push_back(st);
            return true;
        }
    }
    if (static_cast<int>(state->categories.size()) >= state->max_categories) return false;
    SS::CategoryStats cs;
    cs.category = category;
    SS::SummaryStat st;
    st.stat_key = stat_key;
    st.value = value;
    cs.entries.push_back(st);
    state->categories.push_back(cs);
    return true;
}

double SessionSummarySystem::getCategoryStat(const std::string& entity_id,
    const std::string& category, const std::string& stat_key) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& cat : state->categories) {
        if (cat.category == category) {
            for (const auto& s : cat.entries) {
                if (s.stat_key == stat_key) return s.value;
            }
            return 0.0;
        }
    }
    return 0.0;
}

int SessionSummarySystem::getCategoryCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->categories.size()) : 0;
}

bool SessionSummarySystem::finalizeReport(const std::string& entity_id, float end_time) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->finalized) return false;
    state->finalized = true;
    state->session_end_time = end_time;
    return true;
}

bool SessionSummarySystem::isFinalized(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->finalized : false;
}

float SessionSummarySystem::getSessionDuration(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    if (state->finalized) return state->session_end_time - state->session_start_time;
    return state->elapsed_time;
}

double SessionSummarySystem::getIskPerHour(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double isk = getStat(entity_id, "isk_earned");
    float duration = getSessionDuration(entity_id);
    if (duration <= 0.0f) return 0.0;
    return isk / (static_cast<double>(duration) / 3600.0);
}

std::string SessionSummarySystem::getPerformanceGrade(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return "F";
    // Grade based on objectives completed + ISK earned + kills
    double score = 0.0;
    score += getStat(entity_id, "objectives_completed") * 20.0;
    score += getStat(entity_id, "kills") * 10.0;
    score += getStat(entity_id, "isk_earned") / 10000.0;
    if (score >= 100.0) return "S";
    if (score >= 80.0) return "A";
    if (score >= 60.0) return "B";
    if (score >= 40.0) return "C";
    if (score >= 20.0) return "D";
    return "F";
}

double SessionSummarySystem::getTotalDamageDealt(const std::string& entity_id) const {
    return getStat(entity_id, "damage_dealt");
}

double SessionSummarySystem::getTotalDamageReceived(const std::string& entity_id) const {
    return getStat(entity_id, "damage_received");
}

int SessionSummarySystem::getObjectivesCompleted(const std::string& entity_id) const {
    return static_cast<int>(getStat(entity_id, "objectives_completed"));
}

} // namespace systems
} // namespace atlas
