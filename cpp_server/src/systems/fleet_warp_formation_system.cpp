#include "systems/fleet_warp_formation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

#include <cmath>

namespace atlas {
namespace systems {

FleetWarpFormationSystem::FleetWarpFormationSystem(ecs::World* world)
    : System(world) {
}

void FleetWarpFormationSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FleetWarpState>();
    for (auto* entity : entities) {
        auto* ws = entity->getComponent<components::FleetWarpState>();
        if (!ws || !ws->in_fleet_warp) continue;

        // Advance breathing phase
        ws->breathing_phase += delta_time * ws->breathing_frequency * 6.2831853f;
        // Wrap phase to prevent unbounded growth
        if (ws->breathing_phase > 6.2831853f)
            ws->breathing_phase -= 6.2831853f;
    }
}

void FleetWarpFormationSystem::beginFleetWarp(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) {
        entity->addComponent(std::make_unique<components::FleetWarpState>());
        ws = entity->getComponent<components::FleetWarpState>();
    }

    ws->in_fleet_warp = true;
    ws->breathing_phase = 0.0f;
}

void FleetWarpFormationSystem::endFleetWarp(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) return;

    ws->in_fleet_warp = false;
    ws->breathing_phase = 0.0f;
}

bool FleetWarpFormationSystem::isInFleetWarp(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) return false;

    return ws->in_fleet_warp;
}

void FleetWarpFormationSystem::selectFormationByShipClass(
    const std::string& entity_id, const std::string& ship_class) {

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) {
        entity->addComponent(std::make_unique<components::FleetWarpState>());
        ws = entity->getComponent<components::FleetWarpState>();
    }

    ws->ship_class = ship_class;

    // Select formation based on ship class
    if (ship_class == "Frigate" || ship_class == "Destroyer" ||
        ship_class == "Interceptor" || ship_class == "Assault Frigate") {
        ws->warp_formation = components::FleetWarpState::WarpFormationType::TightEchelon;
        ws->breathing_frequency = 0.05f;
        ws->breathing_amplitude = 0.08f;
        ws->distortion_bend = 0.2f;
        ws->wake_ripple = 0.3f;
    } else if (ship_class == "Cruiser" || ship_class == "Battlecruiser" ||
               ship_class == "Heavy Assault Cruiser") {
        ws->warp_formation = components::FleetWarpState::WarpFormationType::LooseDiamond;
        ws->breathing_frequency = 0.03f;
        ws->breathing_amplitude = 0.10f;
        ws->distortion_bend = 0.5f;
        ws->wake_ripple = 0.5f;
    } else {
        // Battleship, Capital, Carrier, Dreadnought, Titan
        ws->warp_formation = components::FleetWarpState::WarpFormationType::WideCapital;
        ws->breathing_frequency = 0.02f;
        ws->breathing_amplitude = 0.12f;
        ws->distortion_bend = 1.0f;
        ws->wake_ripple = 0.8f;
    }
}

float FleetWarpFormationSystem::getBreathingOffset(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) return 0.0f;

    return ws->breathing_amplitude * std::sin(ws->breathing_phase);
}

float FleetWarpFormationSystem::getDistortionBend(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) return 0.0f;

    return ws->distortion_bend;
}

void FleetWarpFormationSystem::computeWarpOffset(
    const std::string& entity_id, int slot_index,
    float& ox, float& oy, float& oz) const {

    auto* entity = world_->getEntity(entity_id);
    if (!entity) { ox = oy = oz = 0.0f; return; }

    auto* ws = entity->getComponent<components::FleetWarpState>();
    if (!ws) { ox = oy = oz = 0.0f; return; }

    if (slot_index == 0) {
        ox = oy = oz = 0.0f;
        return;
    }

    float spacing = kWarpSpacing;
    float breath = ws->breathing_amplitude * std::sin(ws->breathing_phase);

    using WFT = components::FleetWarpState::WarpFormationType;
    switch (ws->warp_formation) {
        case WFT::TightEchelon: {
            // Echelon: staggered line trailing to one side
            int row = slot_index;
            int side = (slot_index % 2 == 1) ? -1 : 1;
            ox = side * ((row + 1) / 2) * spacing * 0.5f + breath * spacing;
            oy = 0.0f;
            oz = -row * spacing * 0.7f;
            break;
        }
        case WFT::LooseDiamond: {
            // Loose diamond spread
            int row = (slot_index + 1) / 2;
            int side = (slot_index % 2 == 1) ? -1 : 1;
            ox = side * row * spacing + breath * spacing;
            oy = 0.0f;
            oz = -row * spacing;
            break;
        }
        case WFT::WideCapital: {
            // Wide spread with slow drift
            int row = (slot_index + 1) / 2;
            int side = (slot_index % 2 == 1) ? -1 : 1;
            ox = side * row * spacing * 1.5f + breath * spacing;
            oy = 0.0f;
            oz = -row * spacing * 0.5f;
            break;
        }
    }
}

} // namespace systems
} // namespace atlas
