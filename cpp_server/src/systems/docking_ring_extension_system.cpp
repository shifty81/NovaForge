#include "systems/docking_ring_extension_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

DockingRingExtensionSystem::DockingRingExtensionSystem(ecs::World* world)
    : System(world) {
}

void DockingRingExtensionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::DockingRingExtension>();
    for (auto* entity : entities) {
        auto* ring = entity->getComponent<components::DockingRingExtension>();
        if (!ring) continue;

        if (!ring->is_powered) continue;

        if (ring->state == components::DockingRingExtension::RingState::Extending) {
            ring->extension_progress += ring->extension_speed * delta_time;
            if (ring->extension_progress >= 1.0f) {
                ring->extension_progress = 1.0f;
                ring->state = components::DockingRingExtension::RingState::Extended;
            }
        } else if (ring->state == components::DockingRingExtension::RingState::Retracting) {
            ring->extension_progress -= ring->extension_speed * delta_time;
            if (ring->extension_progress <= 0.0f) {
                ring->extension_progress = 0.0f;
                ring->state = components::DockingRingExtension::RingState::Retracted;
            }
        }

        // Degrade integrity when connected
        if (ring->is_connected) {
            ring->ring_integrity -= 0.001f * delta_time;
            ring->ring_integrity = std::max(0.0f, ring->ring_integrity);
        }
    }
}

bool DockingRingExtensionSystem::initializeRing(const std::string& entity_id, float ring_diameter) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::DockingRingExtension>();
    if (existing) return false;

    auto comp = std::make_unique<components::DockingRingExtension>();
    comp->ring_diameter = ring_diameter;
    entity->addComponent(std::move(comp));
    return true;
}

bool DockingRingExtensionSystem::extendRing(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    if (!ring->is_powered) return false;
    if (ring->state != components::DockingRingExtension::RingState::Retracted) return false;

    ring->state = components::DockingRingExtension::RingState::Extending;
    return true;
}

bool DockingRingExtensionSystem::retractRing(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    // Disconnect first if connected
    if (ring->is_connected) {
        ring->is_connected = false;
        ring->connected_entity_id.clear();
        ring->pressure_sealed = false;
    }

    if (ring->state == components::DockingRingExtension::RingState::Extended ||
        ring->state == components::DockingRingExtension::RingState::Extending) {
        ring->state = components::DockingRingExtension::RingState::Retracting;
        return true;
    }

    return false;
}

bool DockingRingExtensionSystem::connectRing(const std::string& entity_id,
                                              const std::string& target_entity_id,
                                              components::DockingRingExtension::ConnectionType connection_type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    if (ring->state != components::DockingRingExtension::RingState::Extended) return false;
    if (ring->is_connected) return false;
    if (ring->alignment_angle > ring->alignment_threshold) return false;

    ring->is_connected = true;
    ring->connected_entity_id = target_entity_id;
    ring->connection_type = connection_type;
    ring->total_dockings++;
    return true;
}

bool DockingRingExtensionSystem::disconnectRing(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    if (!ring->is_connected) return false;

    ring->is_connected = false;
    ring->connected_entity_id.clear();
    ring->pressure_sealed = false;
    return true;
}

bool DockingRingExtensionSystem::sealPressure(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    if (ring->state != components::DockingRingExtension::RingState::Extended) return false;
    if (ring->pressure_sealed) return false;

    ring->pressure_sealed = true;
    return true;
}

bool DockingRingExtensionSystem::unsealPressure(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    if (!ring->pressure_sealed) return false;

    ring->pressure_sealed = false;
    return true;
}

bool DockingRingExtensionSystem::setAlignment(const std::string& entity_id, float angle) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    ring->alignment_angle = std::max(0.0f, angle);
    return true;
}

bool DockingRingExtensionSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    ring->is_powered = enabled;
    return true;
}

bool DockingRingExtensionSystem::repairRing(const std::string& entity_id, float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    ring->ring_integrity = std::min(1.0f, ring->ring_integrity + amount);
    return true;
}

std::string DockingRingExtensionSystem::getState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return "unknown";

    return components::DockingRingExtension::stateToString(ring->state);
}

float DockingRingExtensionSystem::getProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return 0.0f;

    return ring->extension_progress;
}

bool DockingRingExtensionSystem::isConnected(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return false;

    return ring->is_connected;
}

float DockingRingExtensionSystem::getIntegrity(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* ring = entity->getComponent<components::DockingRingExtension>();
    if (!ring) return 0.0f;

    return ring->ring_integrity;
}

} // namespace systems
} // namespace atlas
