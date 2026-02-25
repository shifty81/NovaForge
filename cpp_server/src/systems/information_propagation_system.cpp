#include "systems/information_propagation_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

InformationPropagationSystem::InformationPropagationSystem(ecs::World* world)
    : System(world) {
}

void InformationPropagationSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::InformationPropagation>();
    for (auto* entity : entities) {
        auto* info = entity->getComponent<components::InformationPropagation>();
        if (!info) continue;

        // Age and decay rumors
        for (auto& rumor : info->rumors) {
            rumor.age += delta_time;
            if (!rumor.personally_witnessed) {
                rumor.belief_strength -= info->decay_rate * delta_time;
                if (rumor.belief_strength < 0.0f) rumor.belief_strength = 0.0f;
            }
        }

        // Remove expired rumors
        info->rumors.erase(
            std::remove_if(info->rumors.begin(), info->rumors.end(),
                [&](const components::InformationPropagation::Rumor& r) {
                    return r.age > info->max_rumor_age || r.belief_strength <= 0.0f;
                }),
            info->rumors.end());

        // Propagation timer
        info->propagation_timer += delta_time;
        if (info->propagation_timer >= info->propagation_interval) {
            info->propagation_timer = 0.0f;

            // Propagate rumors to neighboring systems
            for (const auto& neighbor_id : info->neighbor_system_ids) {
                auto* neighbor_entity = world_->getEntity(neighbor_id);
                if (!neighbor_entity) continue;
                auto* neighbor_info = neighbor_entity->getComponent<components::InformationPropagation>();
                if (!neighbor_info) continue;

                for (const auto& rumor : info->rumors) {
                    if (rumor.hops >= info->max_hops) continue;
                    if (rumor.belief_strength < 0.1f) continue;

                    // Check if neighbor already has this rumor
                    bool found = false;
                    for (auto& nr : neighbor_info->rumors) {
                        if (nr.rumor_id == rumor.rumor_id) {
                            // Reinforce existing rumor
                            nr.belief_strength = std::min(nr.belief_strength + 0.1f, 1.0f);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        components::InformationPropagation::Rumor propagated = rumor;
                        propagated.belief_strength *= 0.5f;  // halved for second-hand
                        propagated.personally_witnessed = false;
                        propagated.hops += 1;
                        propagated.age = 0.0f;  // Reset age for new system
                        neighbor_info->rumors.push_back(propagated);
                        if (static_cast<int>(neighbor_info->rumors.size()) > neighbor_info->max_rumors) {
                            neighbor_info->rumors.erase(neighbor_info->rumors.begin());
                        }
                    }
                }
            }
        }
    }
}

void InformationPropagationSystem::reportPlayerAction(const std::string& system_id,
                                                       const std::string& player_id,
                                                       const std::string& action_type) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;
    auto* info = entity->getComponent<components::InformationPropagation>();
    if (!info) return;
    std::string rumor_id = player_id + "_" + action_type + "_" + system_id;
    info->addRumor(rumor_id, player_id, action_type, system_id, true);
}

std::vector<components::InformationPropagation::Rumor>
InformationPropagationSystem::getRumors(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return {};
    auto* info = entity->getComponent<components::InformationPropagation>();
    if (!info) return {};
    return info->rumors;
}

std::vector<components::InformationPropagation::Rumor>
InformationPropagationSystem::getRumorsAboutPlayer(const std::string& system_id,
                                                    const std::string& player_id) const {
    std::vector<components::InformationPropagation::Rumor> result;
    auto* entity = world_->getEntity(system_id);
    if (!entity) return result;
    auto* info = entity->getComponent<components::InformationPropagation>();
    if (!info) return result;
    for (const auto& r : info->rumors) {
        if (r.player_id == player_id) result.push_back(r);
    }
    return result;
}

int InformationPropagationSystem::getRumorCount(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0;
    auto* info = entity->getComponent<components::InformationPropagation>();
    if (!info) return 0;
    return info->getRumorCount();
}

float InformationPropagationSystem::getPlayerNotoriety(const std::string& system_id,
                                                        const std::string& player_id) const {
    auto rumors = getRumorsAboutPlayer(system_id, player_id);
    float notoriety = 0.0f;
    for (const auto& r : rumors) {
        notoriety += r.belief_strength;
    }
    return std::min(notoriety, 10.0f);
}

} // namespace systems
} // namespace atlas
