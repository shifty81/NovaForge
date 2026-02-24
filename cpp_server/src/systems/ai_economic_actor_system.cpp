#include "systems/ai_economic_actor_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

AIEconomicActorSystem::AIEconomicActorSystem(ecs::World* world)
    : ecs::System(world) {}

void AIEconomicActorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::AIEconomicActor,
                                         components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* actor  = entity->getComponent<components::AIEconomicActor>();
        auto* intent = entity->getComponent<components::SimNPCIntent>();
        if (!actor || !intent) continue;
        if (actor->permanently_dead) continue;

        // Track time alive
        actor->time_alive += delta_time;

        // If ship is destroyed, attempt replacement
        if (actor->is_destroyed) {
            if (actor->canAffordReplacement(intent->wallet)) {
                intent->wallet -= actor->ship_value;
                actor->is_destroyed = false;
                actor->replacement_count++;
                actor->time_alive = 0.0f;
            } else {
                // Cannot afford — permanently dead
                actor->permanently_dead = true;
            }
        }
    }
}

void AIEconomicActorSystem::earnISK(const std::string& entity_id, double amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* intent = entity->getComponent<components::SimNPCIntent>();
    if (intent && amount > 0.0) {
        intent->wallet += amount;
    }
}

bool AIEconomicActorSystem::spendISK(const std::string& entity_id, double amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* intent = entity->getComponent<components::SimNPCIntent>();
    if (!intent || amount <= 0.0) return false;
    if (intent->wallet < amount) return false;
    intent->wallet -= amount;
    return true;
}

bool AIEconomicActorSystem::handleShipDestruction(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* actor = entity->getComponent<components::AIEconomicActor>();
    if (!actor) return false;

    actor->is_destroyed = true;
    actor->destruction_count++;
    return true;
}

bool AIEconomicActorSystem::canAffordReplacement(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* actor  = entity->getComponent<components::AIEconomicActor>();
    auto* intent = entity->getComponent<components::SimNPCIntent>();
    if (!actor || !intent) return false;
    return actor->canAffordReplacement(intent->wallet);
}

std::vector<std::string> AIEconomicActorSystem::getDeadActors() const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::AIEconomicActor>();
    for (auto* entity : entities) {
        auto* actor = entity->getComponent<components::AIEconomicActor>();
        if (actor && actor->permanently_dead) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

std::vector<std::string> AIEconomicActorSystem::getLowFundsActors(double threshold) const {
    std::vector<std::string> result;
    auto entities = world_->getEntities<components::AIEconomicActor,
                                         components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* intent = entity->getComponent<components::SimNPCIntent>();
        if (intent && intent->wallet < threshold) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

double AIEconomicActorSystem::getTotalEconomyISK() const {
    double total = 0.0;
    auto entities = world_->getEntities<components::AIEconomicActor,
                                         components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* intent = entity->getComponent<components::SimNPCIntent>();
        if (intent) {
            total += intent->wallet;
        }
    }
    return total;
}

} // namespace systems
} // namespace atlas
