#include "systems/achievement_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AchievementSystem::AchievementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void AchievementSystem::updateComponent(ecs::Entity& /*entity*/,
                                         components::AchievementState& comp,
                                         float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool AchievementSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AchievementState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Achievement management
// ---------------------------------------------------------------------------

bool AchievementSystem::addAchievement(
        const std::string& entity_id,
        const std::string& ach_id,
        const std::string& name,
        components::AchievementState::Category category,
        int required_count,
        int reward_points) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ach_id.empty()) return false;
    if (name.empty()) return false;
    if (required_count <= 0) return false;
    if (reward_points < 0) return false;
    if (static_cast<int>(comp->achievements.size()) >= comp->max_achievements) return false;

    for (const auto& a : comp->achievements) {
        if (a.id == ach_id) return false;
    }

    components::AchievementState::Achievement ach;
    ach.id             = ach_id;
    ach.name           = name;
    ach.category       = category;
    ach.required_count = required_count;
    ach.current_count  = 0;
    ach.unlocked       = false;
    ach.reward_points  = reward_points;
    comp->achievements.push_back(ach);
    return true;
}

bool AchievementSystem::removeAchievement(const std::string& entity_id,
                                           const std::string& ach_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->achievements.begin(), comp->achievements.end(),
        [&](const components::AchievementState::Achievement& a) {
            return a.id == ach_id;
        });
    if (it == comp->achievements.end()) return false;
    comp->achievements.erase(it);
    return true;
}

bool AchievementSystem::clearAchievements(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->achievements.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Progression
// ---------------------------------------------------------------------------

bool AchievementSystem::progressAchievement(const std::string& entity_id,
                                             const std::string& ach_id,
                                             int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0) return false;

    auto it = std::find_if(comp->achievements.begin(), comp->achievements.end(),
        [&](const components::AchievementState::Achievement& a) {
            return a.id == ach_id;
        });
    if (it == comp->achievements.end()) return false;
    if (it->unlocked) return false;

    comp->total_progress_calls++;
    it->current_count += amount;
    if (it->current_count >= it->required_count) {
        it->current_count = it->required_count;
        it->unlocked = true;
        comp->total_unlocked++;
        comp->total_reward_points += it->reward_points;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool AchievementSystem::setRewardPoints(const std::string& entity_id,
                                         const std::string& ach_id,
                                         int points) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (points < 0) return false;
    auto it = std::find_if(comp->achievements.begin(), comp->achievements.end(),
        [&](const components::AchievementState::Achievement& a) {
            return a.id == ach_id;
        });
    if (it == comp->achievements.end()) return false;
    if (it->unlocked) return false;  // cannot change reward after unlocked
    it->reward_points = points;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int AchievementSystem::getAchievementCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->achievements.size()) : 0;
}

int AchievementSystem::getUnlockedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_unlocked : 0;
}

bool AchievementSystem::isUnlocked(const std::string& entity_id,
                                    const std::string& ach_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->achievements) {
        if (a.id == ach_id) return a.unlocked;
    }
    return false;
}

int AchievementSystem::getProgress(const std::string& entity_id,
                                    const std::string& ach_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& a : comp->achievements) {
        if (a.id == ach_id) return a.current_count;
    }
    return 0;
}

int AchievementSystem::getRequiredCount(const std::string& entity_id,
                                         const std::string& ach_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& a : comp->achievements) {
        if (a.id == ach_id) return a.required_count;
    }
    return 0;
}

int AchievementSystem::getTotalRewardPoints(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_reward_points : 0;
}

int AchievementSystem::getTotalProgressCalls(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_progress_calls : 0;
}

int AchievementSystem::getCountByCategory(
        const std::string& entity_id,
        components::AchievementState::Category cat) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& a : comp->achievements) {
        if (a.category == cat) count++;
    }
    return count;
}

bool AchievementSystem::hasAchievement(const std::string& entity_id,
                                        const std::string& ach_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->achievements) {
        if (a.id == ach_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
