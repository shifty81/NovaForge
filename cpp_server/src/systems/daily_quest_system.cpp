#include "systems/daily_quest_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DailyQuestSystem::DailyQuestSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void DailyQuestSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::DailyQuestState& comp,
                                        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.reset_timer <= 0.0f) return;

    comp.reset_timer -= delta_time;
    if (comp.reset_timer <= 0.0f) {
        performReset(comp, comp.all_complete);
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool DailyQuestSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DailyQuestState>();
    comp->reset_timer = 0.0f;  // timer not started until objectives are set
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Internal reset helper
// ---------------------------------------------------------------------------

void DailyQuestSystem::performReset(components::DailyQuestState& comp,
                                     bool count_day) {
    if (count_day) {
        comp.days_completed++;
    }
    comp.objectives.clear();
    comp.all_complete   = false;
    comp.bonus_claimed  = false;
    comp.total_resets++;
    comp.reset_timer    = 0.0f;
}

// ---------------------------------------------------------------------------
// Objective management
// ---------------------------------------------------------------------------

bool DailyQuestSystem::addObjective(const std::string& entity_id,
                                     const std::string& obj_id,
                                     const std::string& description,
                                     int required_count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (obj_id.empty()) return false;
    if (required_count <= 0) return false;
    if (static_cast<int>(comp->objectives.size()) >= comp->max_objectives) return false;

    // Blocked while timer is running (quest day in progress)
    if (comp->reset_timer > 0.0f) return false;

    for (const auto& o : comp->objectives) {
        if (o.id == obj_id) return false;
    }

    components::DailyQuestState::DailyObjective obj;
    obj.id             = obj_id;
    obj.description    = description;
    obj.required_count = required_count;
    obj.current_count  = 0;
    obj.completed      = false;
    comp->objectives.push_back(obj);
    return true;
}

bool DailyQuestSystem::removeObjective(const std::string& entity_id,
                                        const std::string& obj_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->objectives.begin(), comp->objectives.end(),
        [&](const components::DailyQuestState::DailyObjective& o) {
            return o.id == obj_id;
        });
    if (it == comp->objectives.end()) return false;
    comp->objectives.erase(it);
    // Recheck all_complete
    comp->all_complete = !comp->objectives.empty() &&
        std::all_of(comp->objectives.begin(), comp->objectives.end(),
            [](const components::DailyQuestState::DailyObjective& o) {
                return o.completed;
            });
    return true;
}

// ---------------------------------------------------------------------------
// Progression
// ---------------------------------------------------------------------------

bool DailyQuestSystem::progressObjective(const std::string& entity_id,
                                          const std::string& obj_id,
                                          int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0) return false;

    auto it = std::find_if(comp->objectives.begin(), comp->objectives.end(),
        [&](const components::DailyQuestState::DailyObjective& o) {
            return o.id == obj_id;
        });
    if (it == comp->objectives.end()) return false;
    if (it->completed) return false;

    // Start the reset timer on first progress call if not already running
    if (comp->reset_timer <= 0.0f) {
        comp->reset_timer = comp->reset_duration;
    }

    it->current_count += amount;
    if (it->current_count >= it->required_count) {
        it->current_count = it->required_count;
        it->completed     = true;
    }

    // Check if all are complete
    comp->all_complete = std::all_of(comp->objectives.begin(), comp->objectives.end(),
        [](const components::DailyQuestState::DailyObjective& o) {
            return o.completed;
        });
    return true;
}

bool DailyQuestSystem::claimBonus(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->all_complete) return false;
    if (comp->bonus_claimed) return false;
    comp->bonus_claimed = true;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool DailyQuestSystem::setBonusReward(const std::string& entity_id,
                                       float reward) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (reward < 0.0f) return false;
    comp->bonus_reward = reward;
    return true;
}

bool DailyQuestSystem::setResetDuration(const std::string& entity_id,
                                         float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (seconds <= 0.0f) return false;
    comp->reset_duration = seconds;
    return true;
}

bool DailyQuestSystem::forceReset(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    performReset(*comp, comp->all_complete);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int DailyQuestSystem::getObjectiveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->objectives.size()) : 0;
}

int DailyQuestSystem::getCompletedObjectiveCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& o : comp->objectives) {
        if (o.completed) count++;
    }
    return count;
}

bool DailyQuestSystem::isObjectiveComplete(const std::string& entity_id,
                                            const std::string& obj_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& o : comp->objectives) {
        if (o.id == obj_id) return o.completed;
    }
    return false;
}

int DailyQuestSystem::getObjectiveProgress(const std::string& entity_id,
                                            const std::string& obj_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& o : comp->objectives) {
        if (o.id == obj_id) return o.current_count;
    }
    return 0;
}

bool DailyQuestSystem::isAllComplete(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->all_complete : false;
}

bool DailyQuestSystem::isBonusClaimed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->bonus_claimed : false;
}

float DailyQuestSystem::getResetTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->reset_timer : 0.0f;
}

float DailyQuestSystem::getBonusReward(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->bonus_reward : 0.0f;
}

int DailyQuestSystem::getDaysCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->days_completed : 0;
}

int DailyQuestSystem::getTotalResets(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_resets : 0;
}

} // namespace systems
} // namespace atlas
