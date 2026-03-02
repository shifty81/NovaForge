#include "systems/planetary_traversal_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

PlanetaryTraversalSystem::PlanetaryTraversalSystem(ecs::World* world)
    : System(world) {
}

void PlanetaryTraversalSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::PlanetaryTraversal>();
    for (auto* entity : entities) {
        auto* trav = entity->getComponent<components::PlanetaryTraversal>();
        if (!trav) continue;

        if (trav->has_destination && trav->is_traversing) {
            float dx = trav->destination_x - trav->position_x;
            float dy = trav->destination_y - trav->position_y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= 1.0f) {
                // Arrived at destination
                trav->position_x = trav->destination_x;
                trav->position_y = trav->destination_y;
                trav->is_traversing = false;
                trav->has_destination = false;
                trav->speed = 0.0f;
            } else {
                // Move toward destination
                float effective_speed = trav->max_speed * trav->terrain_speed_modifier;
                float move_dist = effective_speed * delta_time;
                trav->speed = effective_speed;

                if (move_dist >= dist) {
                    trav->position_x = trav->destination_x;
                    trav->position_y = trav->destination_y;
                    trav->distance_traveled += dist;
                    trav->is_traversing = false;
                    trav->has_destination = false;
                    trav->speed = 0.0f;
                } else {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    trav->position_x += nx * move_dist;
                    trav->position_y += ny * move_dist;
                    trav->distance_traveled += move_dist;
                }
            }
        }
    }
}

bool PlanetaryTraversalSystem::initializeTraversal(const std::string& entity_id,
                                                     const std::string& planet_id,
                                                     float start_x, float start_y) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::PlanetaryTraversal>();
    if (existing) return false;

    auto comp = std::make_unique<components::PlanetaryTraversal>();
    comp->planet_entity_id = planet_id;
    comp->position_x = start_x;
    comp->position_y = start_y;
    entity->addComponent(std::move(comp));
    return true;
}

bool PlanetaryTraversalSystem::removeTraversal(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    entity->removeComponent<components::PlanetaryTraversal>();
    return true;
}

bool PlanetaryTraversalSystem::setDestination(const std::string& entity_id,
                                               float dest_x, float dest_y) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    trav->destination_x = dest_x;
    trav->destination_y = dest_y;
    trav->has_destination = true;
    trav->is_traversing = true;
    return true;
}

bool PlanetaryTraversalSystem::clearDestination(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    trav->has_destination = false;
    trav->is_traversing = false;
    trav->speed = 0.0f;
    return true;
}

bool PlanetaryTraversalSystem::setVehicle(const std::string& entity_id,
                                           const std::string& vehicle_id,
                                           float max_speed) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    trav->vehicle_id = vehicle_id;
    trav->max_speed = max_speed;
    return true;
}

bool PlanetaryTraversalSystem::dismountVehicle(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    trav->vehicle_id.clear();
    trav->max_speed = 5.0f;
    return true;
}

bool PlanetaryTraversalSystem::setTerrainType(const std::string& entity_id,
                                               components::PlanetaryTraversal::TerrainType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    trav->terrain = type;
    trav->terrain_speed_modifier = components::PlanetaryTraversal::getTerrainModifier(type);
    return true;
}

float PlanetaryTraversalSystem::getPositionX(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return 0.0f;

    return trav->position_x;
}

float PlanetaryTraversalSystem::getPositionY(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return 0.0f;

    return trav->position_y;
}

float PlanetaryTraversalSystem::getSpeed(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return 0.0f;

    return trav->speed;
}

float PlanetaryTraversalSystem::getDistanceTraveled(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return 0.0f;

    return trav->distance_traveled;
}

bool PlanetaryTraversalSystem::isTraversing(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return false;

    return trav->is_traversing;
}

float PlanetaryTraversalSystem::getDistanceToDestination(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return 0.0f;

    if (!trav->has_destination) return 0.0f;

    float dx = trav->destination_x - trav->position_x;
    float dy = trav->destination_y - trav->position_y;
    return std::sqrt(dx * dx + dy * dy);
}

std::string PlanetaryTraversalSystem::getTerrainTypeStr(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* trav = entity->getComponent<components::PlanetaryTraversal>();
    if (!trav) return "unknown";

    return components::PlanetaryTraversal::terrainTypeToString(trav->terrain);
}

} // namespace systems
} // namespace atlas
