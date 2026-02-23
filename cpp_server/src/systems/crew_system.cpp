#include "systems/crew_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

CrewSystem::CrewSystem(ecs::World* world)
    : System(world) {
}

void CrewSystem::update(float /*delta_time*/) {
    // Update each CrewMember efficiency_bonus
    auto crew_entities = world_->getEntities<components::CrewMember>();
    for (auto* entity : crew_entities) {
        auto* crew = entity->getComponent<components::CrewMember>();
        if (!crew) continue;
        crew->efficiency_bonus = (crew->skill_level * 0.1f) + (crew->morale / 200.0f);
    }

    // Update each ShipCrew aggregate stats
    auto ship_entities = world_->getEntities<components::ShipCrew>();
    for (auto* entity : ship_entities) {
        auto* ship_crew = entity->getComponent<components::ShipCrew>();
        if (!ship_crew) continue;

        ship_crew->current_crew = static_cast<int>(ship_crew->crew_member_ids.size());

        if (ship_crew->current_crew == 0) {
            ship_crew->overall_efficiency = 1.0f;
            ship_crew->morale_average = 50.0f;
            continue;
        }

        float total_efficiency = 0.0f;
        float total_morale = 0.0f;
        int counted = 0;

        for (const auto& cid : ship_crew->crew_member_ids) {
            auto* crew_entity = world_->getEntity(cid);
            if (!crew_entity) continue;
            auto* crew = crew_entity->getComponent<components::CrewMember>();
            if (!crew) continue;
            total_efficiency += crew->efficiency_bonus;
            total_morale += crew->morale;
            counted++;
        }

        if (counted > 0) {
            ship_crew->overall_efficiency = total_efficiency / static_cast<float>(counted);
            ship_crew->morale_average = total_morale / static_cast<float>(counted);
        }
    }
}

bool CrewSystem::assignCrew(const std::string& ship_entity_id, const std::string& crew_entity_id, const std::string& room_id) {
    auto* ship_entity = world_->getEntity(ship_entity_id);
    if (!ship_entity) return false;
    auto* ship_crew = ship_entity->getComponent<components::ShipCrew>();
    if (!ship_crew) return false;

    if (static_cast<int>(ship_crew->crew_member_ids.size()) >= ship_crew->max_crew) return false;

    auto* crew_entity = world_->getEntity(crew_entity_id);
    if (!crew_entity) return false;
    auto* crew = crew_entity->getComponent<components::CrewMember>();
    if (!crew) return false;

    ship_crew->crew_member_ids.push_back(crew_entity_id);
    ship_crew->current_crew = static_cast<int>(ship_crew->crew_member_ids.size());
    crew->assigned_room_id = room_id;
    crew->current_room_id = room_id;
    return true;
}

bool CrewSystem::removeCrew(const std::string& ship_entity_id, const std::string& crew_entity_id) {
    auto* ship_entity = world_->getEntity(ship_entity_id);
    if (!ship_entity) return false;
    auto* ship_crew = ship_entity->getComponent<components::ShipCrew>();
    if (!ship_crew) return false;

    auto it = std::find(ship_crew->crew_member_ids.begin(), ship_crew->crew_member_ids.end(), crew_entity_id);
    if (it == ship_crew->crew_member_ids.end()) return false;

    ship_crew->crew_member_ids.erase(it);
    ship_crew->current_crew = static_cast<int>(ship_crew->crew_member_ids.size());
    return true;
}

int CrewSystem::getCrewCount(const std::string& ship_entity_id) const {
    auto* ship_entity = world_->getEntity(ship_entity_id);
    if (!ship_entity) return 0;
    auto* ship_crew = ship_entity->getComponent<components::ShipCrew>();
    if (!ship_crew) return 0;
    return ship_crew->current_crew;
}

float CrewSystem::getOverallEfficiency(const std::string& ship_entity_id) const {
    auto* ship_entity = world_->getEntity(ship_entity_id);
    if (!ship_entity) return 1.0f;
    auto* ship_crew = ship_entity->getComponent<components::ShipCrew>();
    if (!ship_crew) return 1.0f;
    return ship_crew->overall_efficiency;
}

void CrewSystem::setActivity(const std::string& crew_entity_id, components::CrewMember::Activity activity) {
    auto* entity = world_->getEntity(crew_entity_id);
    if (!entity) return;
    auto* crew = entity->getComponent<components::CrewMember>();
    if (!crew) return;
    crew->current_activity = activity;
}

} // namespace systems
} // namespace atlas
