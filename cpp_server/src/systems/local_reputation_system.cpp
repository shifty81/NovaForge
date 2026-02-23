#include "systems/local_reputation_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

LocalReputationSystem::LocalReputationSystem(ecs::World* world)
    : System(world) {
}

void LocalReputationSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::LocalReputation>();
    for (auto* entity : entities) {
        auto* rep = entity->getComponent<components::LocalReputation>();
        if (!rep) continue;

        // Decay all player reputations toward 0
        for (auto& pair : rep->player_reputation) {
            if (pair.second > 0.0f) {
                pair.second -= rep->reputation_decay_rate * delta_time;
                if (pair.second < 0.0f) pair.second = 0.0f;
            } else if (pair.second < 0.0f) {
                pair.second += rep->reputation_decay_rate * delta_time;
                if (pair.second > 0.0f) pair.second = 0.0f;
            }
        }
    }
}

void LocalReputationSystem::modifyReputation(const std::string& system_id,
                                              const std::string& player_id,
                                              float amount) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;
    auto* rep = entity->getComponent<components::LocalReputation>();
    if (!rep) return;
    rep->player_reputation[player_id] = std::clamp(
        rep->player_reputation[player_id] + amount, -100.0f, 100.0f);
}

float LocalReputationSystem::getReputation(const std::string& system_id,
                                            const std::string& player_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.0f;
    auto* rep = entity->getComponent<components::LocalReputation>();
    if (!rep) return 0.0f;
    auto it = rep->player_reputation.find(player_id);
    if (it == rep->player_reputation.end()) return 0.0f;
    return it->second;
}

std::string LocalReputationSystem::getStanding(const std::string& system_id,
                                                const std::string& player_id) const {
    float val = getReputation(system_id, player_id);
    if (val < -50.0f) return "Hostile";
    if (val < -10.0f) return "Unfriendly";
    if (val > 50.0f) return "Allied";
    if (val > 10.0f) return "Friendly";
    return "Neutral";
}

} // namespace systems
} // namespace atlas
