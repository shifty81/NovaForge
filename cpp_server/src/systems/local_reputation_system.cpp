#include "systems/local_reputation_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

LocalReputationSystem::LocalReputationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void LocalReputationSystem::updateComponent(ecs::Entity& /*entity*/, components::LocalReputation& rep, float delta_time) {
    // Decay all player reputations toward 0
    for (auto& pair : rep.player_reputation) {
        if (pair.second > 0.0f) {
            pair.second -= rep.reputation_decay_rate * delta_time;
            if (pair.second < 0.0f) pair.second = 0.0f;
        } else if (pair.second < 0.0f) {
            pair.second += rep.reputation_decay_rate * delta_time;
            if (pair.second > 0.0f) pair.second = 0.0f;
        }
    }
}

void LocalReputationSystem::modifyReputation(const std::string& system_id,
                                              const std::string& player_id,
                                              float amount) {
    auto* rep = getComponentFor(system_id);
    if (!rep) return;
    rep->player_reputation[player_id] = std::clamp(
        rep->player_reputation[player_id] + amount, -100.0f, 100.0f);
}

float LocalReputationSystem::getReputation(const std::string& system_id,
                                            const std::string& player_id) const {
    const auto* rep = getComponentFor(system_id);
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
