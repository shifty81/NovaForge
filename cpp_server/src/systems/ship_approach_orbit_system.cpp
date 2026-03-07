#include "systems/ship_approach_orbit_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ShipApproachOrbitSystem::ShipApproachOrbitSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipApproachOrbitSystem::updateComponent(ecs::Entity& entity,
    components::ApproachOrbitState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.command_type == "none") return;

    if (comp.command_type == "approach") {
        // Close distance toward target at max speed
        if (comp.current_distance > comp.desired_distance) {
            float close_rate = std::min(comp.max_speed * delta_time,
                                        comp.current_distance - comp.desired_distance);
            comp.current_distance -= close_rate;
            comp.current_speed = close_rate / delta_time;
        } else {
            comp.current_speed = 0.0f;
        }
    } else if (comp.command_type == "orbit") {
        // Maintain orbit at desired_distance, advancing angle
        float diff = comp.current_distance - comp.desired_distance;
        if (std::fabs(diff) > 1.0f) {
            float correction = std::min(comp.max_speed * delta_time, std::fabs(diff));
            comp.current_distance -= (diff > 0 ? correction : -correction);
        }
        // Advance orbit angle based on speed and radius
        float circumference = 2.0f * 3.14159265f * comp.desired_distance;
        if (circumference > 0.0f) {
            float angular_speed = comp.max_speed / circumference * 360.0f;
            comp.orbit_angle += angular_speed * delta_time;
            if (comp.orbit_angle >= 360.0f) comp.orbit_angle -= 360.0f;
        }
        comp.current_speed = comp.max_speed;
    } else if (comp.command_type == "keep_at_range") {
        // Maintain fixed distance — move closer or further as needed
        float diff = comp.current_distance - comp.desired_distance;
        if (std::fabs(diff) > 0.5f) {
            float correction = std::min(comp.max_speed * delta_time, std::fabs(diff));
            comp.current_distance -= (diff > 0 ? correction : -correction);
            comp.current_speed = correction / delta_time;
        } else {
            comp.current_speed = 0.0f;
        }
    }
}

bool ShipApproachOrbitSystem::initialize(const std::string& entity_id,
    float max_speed) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ApproachOrbitState>();
    comp->max_speed = std::max(0.0f, max_speed);
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipApproachOrbitSystem::commandApproach(const std::string& entity_id,
    const std::string& target_id, float target_distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->command_type = "approach";
    comp->target_id = target_id;
    comp->current_distance = std::max(0.0f, target_distance);
    comp->desired_distance = 0.0f;
    comp->orbit_angle = 0.0f;
    comp->current_speed = 0.0f;
    return true;
}

bool ShipApproachOrbitSystem::commandOrbit(const std::string& entity_id,
    const std::string& target_id, float orbit_radius) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (orbit_radius <= 0.0f) return false;
    comp->command_type = "orbit";
    comp->target_id = target_id;
    comp->desired_distance = orbit_radius;
    comp->orbit_angle = 0.0f;
    comp->current_speed = 0.0f;
    return true;
}

bool ShipApproachOrbitSystem::commandKeepAtRange(const std::string& entity_id,
    const std::string& target_id, float desired_range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (desired_range < 0.0f) return false;
    comp->command_type = "keep_at_range";
    comp->target_id = target_id;
    comp->desired_distance = desired_range;
    comp->orbit_angle = 0.0f;
    comp->current_speed = 0.0f;
    return true;
}

bool ShipApproachOrbitSystem::stopCommand(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->command_type == "none") return false;
    comp->command_type = "none";
    comp->target_id.clear();
    comp->current_speed = 0.0f;
    comp->orbit_angle = 0.0f;
    return true;
}

bool ShipApproachOrbitSystem::setMaxSpeed(const std::string& entity_id,
    float max_speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->max_speed = std::max(0.0f, max_speed);
    return true;
}

std::string ShipApproachOrbitSystem::getCommandType(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->command_type : "none";
}

std::string ShipApproachOrbitSystem::getTargetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->target_id : "";
}

float ShipApproachOrbitSystem::getCurrentDistance(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_distance : 0.0f;
}

float ShipApproachOrbitSystem::getDesiredDistance(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->desired_distance : 0.0f;
}

float ShipApproachOrbitSystem::getCurrentSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_speed : 0.0f;
}

float ShipApproachOrbitSystem::getOrbitAngle(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->orbit_angle : 0.0f;
}

bool ShipApproachOrbitSystem::isCommandActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->command_type != "none") : false;
}

} // namespace systems
} // namespace atlas
