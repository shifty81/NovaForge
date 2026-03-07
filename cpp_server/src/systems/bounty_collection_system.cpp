#include "systems/bounty_collection_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

BountyCollectionSystem::BountyCollectionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool BountyCollectionSystem::initializeBountyTracker(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::BountyCollection>()) return false;

    auto bc = std::make_unique<components::BountyCollection>();
    entity->addComponent(std::move(bc));
    return true;
}

bool BountyCollectionSystem::recordKill(const std::string& entity_id,
                                         const std::string& target_id,
                                         const std::string& target_type,
                                         double bounty_amount) {
    auto* bc = getComponentFor(entity_id);
    if (!bc || !bc->active) return false;
    if (target_id.empty() || bounty_amount <= 0.0) return false;
    if (static_cast<int>(bc->pending_kills.size()) >= bc->max_pending) return false;

    components::BountyCollection::BountyKill kill;
    kill.target_id = target_id;
    kill.target_type = target_type;
    kill.bounty_amount = bounty_amount;
    kill.timestamp = bc->elapsed;
    bc->pending_kills.push_back(kill);
    return true;
}

int BountyCollectionSystem::getPendingCount(const std::string& entity_id) const {
    auto* bc = getComponentFor(entity_id);
    if (!bc) return 0;
    int count = 0;
    for (const auto& k : bc->pending_kills) {
        if (!k.paid) ++count;
    }
    return count;
}

double BountyCollectionSystem::getTotalCollected(const std::string& entity_id) const {
    auto* bc = getComponentFor(entity_id);
    return bc ? bc->total_bounties_collected : 0.0;
}

int BountyCollectionSystem::getTotalKillsClaimed(const std::string& entity_id) const {
    auto* bc = getComponentFor(entity_id);
    return bc ? bc->total_kills_claimed : 0;
}

bool BountyCollectionSystem::hasPendingBounties(const std::string& entity_id) const {
    auto* bc = getComponentFor(entity_id);
    if (!bc) return false;
    for (const auto& k : bc->pending_kills) {
        if (!k.paid) return true;
    }
    return false;
}

bool BountyCollectionSystem::clearPending(const std::string& entity_id) {
    auto* bc = getComponentFor(entity_id);
    if (!bc) return false;
    bc->pending_kills.clear();
    return true;
}

void BountyCollectionSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::BountyCollection& bc,
                                              float delta_time) {
    if (!bc.active) return;
    bc.elapsed += delta_time;

    for (auto& kill : bc.pending_kills) {
        if (kill.paid) continue;
        kill.paid = true;
        bc.total_bounties_collected += kill.bounty_amount;
        bc.total_kills_claimed++;
    }
}

} // namespace systems
} // namespace atlas
