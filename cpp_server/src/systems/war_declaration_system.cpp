#include "systems/war_declaration_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

WarDeclarationSystem::WarDeclarationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void WarDeclarationSystem::updateComponent(ecs::Entity& entity, components::WarDeclaration& war, float delta_time) {
    float dt_hours = delta_time / 3600.0f;

    if (war.status == "active" || war.status == "mutual") {
        war.elapsed_hours += dt_hours;
        // Mutual wars have no time limit; only non-mutual wars auto-finish
        if (!war.is_mutual && war.elapsed_hours >= war.duration_hours) {
            war.status = "finished";
        }
    }
}

std::string WarDeclarationSystem::declareWar(const std::string& aggressor_id,
                                             const std::string& defender_id,
                                             double war_cost) {
    auto* aggressor_entity = world_->getEntity(aggressor_id);
    if (!aggressor_entity) return "";

    auto* player = aggressor_entity->getComponent<components::Player>();
    if (!player) return "";

    if (player->credits < war_cost) return "";

    player->credits -= war_cost;

    war_counter_++;
    std::string war_id = "war_" + std::to_string(war_counter_);

    auto* war_entity = world_->createEntity(war_id);
    if (!war_entity) return "";

    auto war = std::make_unique<components::WarDeclaration>();
    war->war_id = war_id;
    war->aggressor_id = aggressor_id;
    war->defender_id = defender_id;
    war->war_cost = war_cost;
    war->status = "pending";
    war_entity->addComponent(std::move(war));

    return war_id;
}

bool WarDeclarationSystem::activateWar(const std::string& war_entity_id) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return false;

    if (war->status != "pending") return false;

    war->status = "active";
    return true;
}

bool WarDeclarationSystem::makeMutual(const std::string& war_entity_id,
                                      const std::string& requester_id) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return false;

    // Only defender can make mutual
    if (war->defender_id != requester_id) return false;

    war->is_mutual = true;
    war->status = "mutual";
    return true;
}

bool WarDeclarationSystem::surrender(const std::string& war_entity_id,
                                     const std::string& requester_id) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return false;

    // Only defender can surrender
    if (war->defender_id != requester_id) return false;

    war->status = "surrendered";
    return true;
}

bool WarDeclarationSystem::retractWar(const std::string& war_entity_id,
                                      const std::string& requester_id) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return false;

    // Only aggressor can retract
    if (war->aggressor_id != requester_id) return false;

    war->status = "retracted";
    return true;
}

bool WarDeclarationSystem::recordKill(const std::string& war_entity_id,
                                      const std::string& killer_side,
                                      double isk_value) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return false;

    if (killer_side == "aggressor") {
        war->aggressor_kills++;
        war->aggressor_isk_destroyed += isk_value;
    } else if (killer_side == "defender") {
        war->defender_kills++;
        war->defender_isk_destroyed += isk_value;
    } else {
        return false;
    }

    return true;
}

std::string WarDeclarationSystem::getWarStatus(const std::string& war_entity_id) {
    auto* war = getComponentFor(war_entity_id);
    if (!war) return "";

    return war->status;
}

} // namespace systems
} // namespace atlas
