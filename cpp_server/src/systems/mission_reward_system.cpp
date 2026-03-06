#include "systems/mission_reward_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

MissionRewardSystem::MissionRewardSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MissionRewardSystem::updateComponent(ecs::Entity& /*entity*/,
    components::MissionReward& reward, float delta_time) {
    if (!reward.active) return;
    reward.elapsed += delta_time;
}

bool MissionRewardSystem::addReward(const std::string& entity_id,
    const std::string& mission_id, double isk_amount, const std::string& faction_id,
    double standing_change, const std::string& item_id, int item_quantity) {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return false;

    // Check for duplicate mission_id
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return false;
    }

    // Enforce max pending
    int pending = 0;
    for (const auto& r : reward->rewards) {
        if (!r.collected) pending++;
    }
    if (pending >= reward->max_pending) return false;

    components::MissionReward::RewardEntry entry;
    entry.mission_id = mission_id;
    entry.isk_amount = isk_amount;
    entry.faction_id = faction_id;
    entry.standing_change = standing_change;
    entry.item_id = item_id;
    entry.item_quantity = item_quantity;
    reward->rewards.push_back(entry);
    return true;
}

bool MissionRewardSystem::collectReward(const std::string& entity_id,
    const std::string& mission_id) {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return false;

    for (auto& r : reward->rewards) {
        if (r.mission_id == mission_id) {
            if (r.collected) return false;  // Already collected
            r.collected = true;
            r.collected_at = reward->elapsed;
            reward->total_collected++;
            reward->total_isk_earned += r.isk_amount;
            reward->total_standing_gained += r.standing_change;
            return true;
        }
    }
    return false;
}

bool MissionRewardSystem::isCollected(const std::string& entity_id,
    const std::string& mission_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return false;
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return r.collected;
    }
    return false;
}

int MissionRewardSystem::getPendingCount(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return 0;
    int count = 0;
    for (const auto& r : reward->rewards) {
        if (!r.collected) count++;
    }
    return count;
}

int MissionRewardSystem::getTotalCollected(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_collected : 0;
}

double MissionRewardSystem::getTotalIskEarned(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_isk_earned : 0.0;
}

double MissionRewardSystem::getTotalStandingGained(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_standing_gained : 0.0;
}

double MissionRewardSystem::getRewardIsk(const std::string& entity_id,
    const std::string& mission_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return 0.0;
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return r.isk_amount;
    }
    return 0.0;
}

std::string MissionRewardSystem::getRewardItemId(const std::string& entity_id,
    const std::string& mission_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return "";
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return r.item_id;
    }
    return "";
}

int MissionRewardSystem::getRewardItemQuantity(const std::string& entity_id,
    const std::string& mission_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return 0;
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return r.item_quantity;
    }
    return 0;
}

bool MissionRewardSystem::hasReward(const std::string& entity_id,
    const std::string& mission_id) const {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return false;
    for (const auto& r : reward->rewards) {
        if (r.mission_id == mission_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
