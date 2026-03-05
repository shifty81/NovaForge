#include "systems/station_monument_system.h"
#include "systems/legend_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

StationMonumentSystem::StationMonumentSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void StationMonumentSystem::updateComponent(ecs::Entity& /*entity*/,
    components::StationMonument& /*monument*/, float /*delta_time*/) {
    // Monument creation is event-driven via checkAndCreateMonument
}

std::string StationMonumentSystem::checkAndCreateMonument(
    const std::string& station_entity_id,
    const std::string& player_entity_id,
    float timestamp) {

    auto* station = world_->getEntity(station_entity_id);
    if (!station) return "";

    auto* player_entity = world_->getEntity(player_entity_id);
    if (!player_entity) return "";

    auto* legend = player_entity->getComponent<components::PlayerLegend>();
    if (!legend || legend->legend_score < kMonumentMinScore) return "";

    // Check if a monument for this player already exists in this station
    for (auto* entity : world_->getEntities<components::StationMonument>()) {
        auto* monument = entity->getComponent<components::StationMonument>();
        if (!monument) continue;
        if (monument->station_id == station_entity_id &&
            monument->player_id == player_entity_id) {
            // Upgrade existing monument if score has grown
            auto new_type = components::StationMonument::scoreToType(legend->legend_score);
            if (new_type > monument->type) {
                monument->type = new_type;
                monument->legend_score_at_creation = legend->legend_score;
                monument->inscription =
                    "In honour of " + monument->player_name +
                    ", " + components::StationMonument::typeToString(new_type) +
                    " — Legend Score: " + std::to_string(legend->legend_score);
                return entity->getId(); // return existing id on upgrade
            }
            return ""; // no change needed
        }
    }

    // Determine player name from Player component if available
    std::string player_name = player_entity_id;
    auto* player_comp = player_entity->getComponent<components::Player>();
    if (player_comp && !player_comp->player_id.empty()) {
        player_name = player_comp->player_id;
    }

    auto monument_type = components::StationMonument::scoreToType(legend->legend_score);
    std::string monument_id = "monument_" + station_entity_id + "_" +
                              std::to_string(++monument_counter_);
    auto* monument_entity = world_->createEntity(monument_id);
    if (!monument_entity) return "";

    auto monument = std::make_unique<components::StationMonument>();
    monument->station_id = station_entity_id;
    monument->player_id = player_entity_id;
    monument->player_name = player_name;
    monument->type = monument_type;
    monument->legend_score_at_creation = legend->legend_score;
    monument->creation_timestamp = timestamp;
    monument->inscription =
        "In honour of " + player_name + ", " +
        components::StationMonument::typeToString(monument_type) +
        " — Legend Score: " + std::to_string(legend->legend_score);
    monument_entity->addComponent(std::move(monument));
    return monument_id;
}

int StationMonumentSystem::getMonumentCount(const std::string& station_entity_id) const {
    int count = 0;
    for (auto* entity : world_->getEntities<components::StationMonument>()) {
        auto* monument = entity->getComponent<components::StationMonument>();
        if (monument && monument->station_id == station_entity_id) {
            ++count;
        }
    }
    return count;
}

std::string StationMonumentSystem::getMonumentType(const std::string& station_entity_id,
                                                    const std::string& player_id) const {
    for (auto* entity : world_->getEntities<components::StationMonument>()) {
        auto* monument = entity->getComponent<components::StationMonument>();
        if (!monument) continue;
        if (monument->station_id == station_entity_id &&
            monument->player_id == player_id) {
            return components::StationMonument::typeToString(monument->type);
        }
    }
    return "None";
}

std::vector<std::string> StationMonumentSystem::getMonuments(
    const std::string& station_entity_id) const {
    std::vector<std::string> result;
    for (auto* entity : world_->getEntities<components::StationMonument>()) {
        auto* monument = entity->getComponent<components::StationMonument>();
        if (monument && monument->station_id == station_entity_id) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
