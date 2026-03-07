#ifndef NOVAFORGE_SYSTEMS_BOUNTY_COLLECTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BOUNTY_COLLECTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes bounty claims when NPCs with bounties are destroyed
 *
 * Records kills, awards credits from bounties, and tracks total collection
 * statistics per player entity.
 */
class BountyCollectionSystem : public ecs::SingleComponentSystem<components::BountyCollection> {
public:
    explicit BountyCollectionSystem(ecs::World* world);
    ~BountyCollectionSystem() override = default;

    std::string getName() const override { return "BountyCollectionSystem"; }

    bool initializeBountyTracker(const std::string& entity_id);
    bool recordKill(const std::string& entity_id, const std::string& target_id,
                    const std::string& target_type, double bounty_amount);
    int getPendingCount(const std::string& entity_id) const;
    double getTotalCollected(const std::string& entity_id) const;
    int getTotalKillsClaimed(const std::string& entity_id) const;
    bool hasPendingBounties(const std::string& entity_id) const;
    bool clearPending(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::BountyCollection& bc,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BOUNTY_COLLECTION_SYSTEM_H
