#include "systems/convoy_ambush_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ConvoyAmbushSystem::ConvoyAmbushSystem(ecs::World* world)
    : System(world) {
}

void ConvoyAmbushSystem::update(float delta_time) {
    // Tick cooldowns on active/finished ambushes
    for (auto* entity : world_->getEntities<components::ConvoyAmbush>()) {
        auto* ambush = entity->getComponent<components::ConvoyAmbush>();
        if (!ambush) continue;
        if (ambush->cooldown_remaining > 0.0f) {
            ambush->cooldown_remaining -= delta_time;
            if (ambush->cooldown_remaining < 0.0f)
                ambush->cooldown_remaining = 0.0f;
        }
    }
    // Update pirate interest on routes (grows with cargo value, decays slowly)
    for (auto* entity : world_->getEntities<components::ConvoyRoute>()) {
        auto* route = entity->getComponent<components::ConvoyRoute>();
        if (!route) continue;
        float target_interest = std::min(1.0f,
            static_cast<float>(route->cargo_value / 1000000000.0)); // 1B = full interest
        // Decay toward target interest slowly
        float diff = target_interest - route->pirate_interest;
        route->pirate_interest += diff * delta_time * 0.001f;
    }
}

std::string ConvoyAmbushSystem::registerRoute(const std::string& origin,
                                               const std::string& destination,
                                               const std::string& cargo_type,
                                               double cargo_value,
                                               float security_level) {
    std::string route_id = "route_" + std::to_string(++route_counter_);
    auto* entity = world_->createEntity(route_id);
    if (!entity) return "";

    auto route = std::make_unique<components::ConvoyRoute>();
    route->route_id = route_id;
    route->origin_system = origin;
    route->destination_system = destination;
    route->cargo_type = cargo_type;
    route->cargo_value = cargo_value;
    route->security_level = security_level;
    route->pirate_interest = std::min(1.0f,
        static_cast<float>(cargo_value / 1000000000.0));
    entity->addComponent(std::move(route));
    return route_id;
}

std::string ConvoyAmbushSystem::planAmbush(const std::string& pirate_entity_id,
                                            const std::string& route_entity_id) {
    auto* pirate = world_->getEntity(pirate_entity_id);
    if (!pirate) return "";

    auto* route_entity = world_->getEntity(route_entity_id);
    if (!route_entity) return "";
    auto* route = route_entity->getComponent<components::ConvoyRoute>();
    if (!route) return "";

    std::string ambush_id = "ambush_" + std::to_string(++ambush_counter_);
    auto* ambush_entity = world_->createEntity(ambush_id);
    if (!ambush_entity) return "";

    auto ambush = std::make_unique<components::ConvoyAmbush>();
    ambush->ambush_id = ambush_id;
    ambush->pirate_entity_id = pirate_entity_id;
    ambush->route_id = route_entity_id;
    ambush->state = components::ConvoyAmbush::AmbushState::Planned;
    ambush_entity->addComponent(std::move(ambush));
    return ambush_id;
}

bool ConvoyAmbushSystem::executeAmbush(const std::string& ambush_entity_id) {
    auto* entity = world_->getEntity(ambush_entity_id);
    if (!entity) return false;
    auto* ambush = entity->getComponent<components::ConvoyAmbush>();
    if (!ambush) return false;
    if (ambush->state != components::ConvoyAmbush::AmbushState::Planned) return false;

    // Check route security — high-sec routes fail ambushes
    auto* route_entity = world_->getEntity(ambush->route_id);
    float security = 0.0f;
    double cargo_value = 0.0;
    if (route_entity) {
        auto* route = route_entity->getComponent<components::ConvoyRoute>();
        if (route) {
            security = route->security_level;
            cargo_value = route->cargo_value;
            route->active_convoys++;
        }
    }

    // Ambush succeeds when security < 0.7
    if (security >= 0.7f) {
        ambush->state = components::ConvoyAmbush::AmbushState::Failed;
        return false;
    }

    ambush->state = components::ConvoyAmbush::AmbushState::Active;
    ambush->ships_attacked = 1;
    // Loot value is a fraction of cargo depending on how lawless the route is
    ambush->loot_value = cargo_value * (1.0 - static_cast<double>(security)) * 0.5;
    return true;
}

bool ConvoyAmbushSystem::disperseAmbush(const std::string& ambush_entity_id) {
    auto* entity = world_->getEntity(ambush_entity_id);
    if (!entity) return false;
    auto* ambush = entity->getComponent<components::ConvoyAmbush>();
    if (!ambush) return false;
    if (ambush->state != components::ConvoyAmbush::AmbushState::Active) return false;

    ambush->state = components::ConvoyAmbush::AmbushState::Dispersed;
    ambush->cooldown_remaining = 3600.0f; // 1-hour cooldown
    return true;
}

std::string ConvoyAmbushSystem::getAmbushState(const std::string& ambush_entity_id) const {
    auto* entity = world_->getEntity(ambush_entity_id);
    if (!entity) return "unknown";
    auto* ambush = entity->getComponent<components::ConvoyAmbush>();
    if (!ambush) return "unknown";
    return components::ConvoyAmbush::stateToString(ambush->state);
}

float ConvoyAmbushSystem::getRouteRisk(const std::string& route_entity_id) const {
    auto* entity = world_->getEntity(route_entity_id);
    if (!entity) return 0.0f;
    auto* route = entity->getComponent<components::ConvoyRoute>();
    if (!route) return 0.0f;
    return route->pirate_interest;
}

std::vector<std::string> ConvoyAmbushSystem::getPlannedAmbushes() const {
    std::vector<std::string> result;
    for (auto* entity : world_->getEntities<components::ConvoyAmbush>()) {
        auto* ambush = entity->getComponent<components::ConvoyAmbush>();
        if (ambush && ambush->state == components::ConvoyAmbush::AmbushState::Planned) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
