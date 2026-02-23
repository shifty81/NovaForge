#include "systems/wreck_persistence_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

WreckPersistenceSystem::WreckPersistenceSystem(ecs::World* world)
    : System(world) {
}

void WreckPersistenceSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::WreckPersistence>();
    for (auto* entity : entities) {
        auto* wreck = entity->getComponent<components::WreckPersistence>();
        if (!wreck) continue;

        wreck->elapsed += delta_time;

        // Mark expired wrecks
        if (wreck->elapsed >= wreck->lifetime) {
            wreck->lifetime = -1.0f;
        }

        // Try to assign salvage NPC
        if (!wreck->salvage_npc_assigned) {
            auto npc_entities = world_->getEntities<components::SimNPCIntent>();
            for (auto* npc : npc_entities) {
                auto* intent = npc->getComponent<components::SimNPCIntent>();
                if (intent && intent->current_intent == components::SimNPCIntent::Intent::Salvage) {
                    wreck->salvage_npc_assigned = true;
                    wreck->assigned_npc_id = npc->getId();
                    break;
                }
            }
        }
    }
}

bool WreckPersistenceSystem::isExpired(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* wreck = entity->getComponent<components::WreckPersistence>();
    if (!wreck) return false;
    return wreck->lifetime < 0.0f;
}

float WreckPersistenceSystem::getRemainingLifetime(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* wreck = entity->getComponent<components::WreckPersistence>();
    if (!wreck) return 0.0f;
    if (wreck->lifetime < 0.0f) return 0.0f;
    float remaining = wreck->lifetime - wreck->elapsed;
    return remaining > 0.0f ? remaining : 0.0f;
}

void WreckPersistenceSystem::assignSalvageNPC(const std::string& wreck_id,
                                                const std::string& npc_id) {
    auto* entity = world_->getEntity(wreck_id);
    if (!entity) return;
    auto* wreck = entity->getComponent<components::WreckPersistence>();
    if (!wreck) return;
    wreck->salvage_npc_assigned = true;
    wreck->assigned_npc_id = npc_id;
}

std::vector<std::string> WreckPersistenceSystem::getExpiredWrecks() const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::WreckPersistence>();
    for (auto* entity : entities) {
        auto* wreck = entity->getComponent<components::WreckPersistence>();
        if (wreck && wreck->lifetime < 0.0f) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
