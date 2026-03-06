#include "systems/combat_reward_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

CombatRewardSystem::CombatRewardSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool CombatRewardSystem::initializeRewards(const std::string& entity_id,
                                           float flush_interval) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::CombatReward>()) return false;

    auto reward = std::make_unique<components::CombatReward>();
    reward->flush_interval = flush_interval;
    entity->addComponent(std::move(reward));
    return true;
}

bool CombatRewardSystem::recordKill(const std::string& entity_id,
                                    const std::string& target_id,
                                    const std::string& target_name,
                                    float xp,
                                    double credits,
                                    const std::string& loot_table) {
    auto* reward = getComponentFor(entity_id);
    if (!reward || !reward->active) return false;
    if (target_id.empty()) return false;

    components::CombatReward::KillReward kr;
    kr.target_id = target_id;
    kr.target_name = target_name;
    kr.xp_awarded = xp;
    kr.credits_awarded = credits;
    kr.loot_table = loot_table;
    kr.timestamp = reward->elapsed;

    reward->pending_rewards.push_back(kr);
    reward->pending_count++;
    reward->total_kills++;
    return true;
}

double CombatRewardSystem::getTotalCredits(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_credits_awarded : 0.0;
}

float CombatRewardSystem::getTotalXP(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_xp_awarded : 0.0f;
}

int CombatRewardSystem::getTotalKills(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->total_kills : 0;
}

int CombatRewardSystem::getPendingCount(const std::string& entity_id) const {
    auto* reward = getComponentFor(entity_id);
    return reward ? reward->pending_count : 0;
}

int CombatRewardSystem::flushRewards(const std::string& entity_id) {
    auto* reward = getComponentFor(entity_id);
    if (!reward) return 0;

    int flushed = 0;
    for (auto& kr : reward->pending_rewards) {
        if (kr.flushed) continue;
        kr.flushed = true;
        reward->total_credits_awarded += kr.credits_awarded;
        reward->total_xp_awarded += kr.xp_awarded;
        flushed++;
    }
    reward->pending_count = 0;
    reward->time_since_flush = 0.0f;
    return flushed;
}

void CombatRewardSystem::updateComponent(ecs::Entity& /*entity*/,
                                         components::CombatReward& reward,
                                         float delta_time) {
    if (!reward.active) return;
    reward.elapsed += delta_time;
    reward.time_since_flush += delta_time;

    // Auto-flush when interval elapses and there are pending rewards
    if (reward.time_since_flush >= reward.flush_interval && reward.pending_count > 0) {
        for (auto& kr : reward.pending_rewards) {
            if (kr.flushed) continue;
            kr.flushed = true;
            reward.total_credits_awarded += kr.credits_awarded;
            reward.total_xp_awarded += kr.xp_awarded;
        }
        reward.pending_count = 0;
        reward.time_since_flush = 0.0f;
    }
}

} // namespace systems
} // namespace atlas
