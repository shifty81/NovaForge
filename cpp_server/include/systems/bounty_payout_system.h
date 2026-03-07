#ifndef NOVAFORGE_SYSTEMS_BOUNTY_PAYOUT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BOUNTY_PAYOUT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Automatic bounty rewards on NPC destruction
 *
 * Queues payout entries when an NPC is killed and processes them
 * each tick.  Supports a configurable payout multiplier derived
 * from security status or faction standings.
 */
class BountyPayoutSystem : public ecs::SingleComponentSystem<components::BountyPayout> {
public:
    explicit BountyPayoutSystem(ecs::World* world);
    ~BountyPayoutSystem() override = default;

    std::string getName() const override { return "BountyPayoutSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool recordKill(const std::string& entity_id, const std::string& killer_id,
                    const std::string& victim_id, const std::string& victim_type,
                    double base_bounty);
    bool setPayoutMultiplier(const std::string& entity_id, float multiplier);

    int getPendingCount(const std::string& entity_id) const;
    double getTotalIscPaid(const std::string& entity_id) const;
    int getTotalPayoutsProcessed(const std::string& entity_id) const;
    float getPayoutMultiplier(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::BountyPayout& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BOUNTY_PAYOUT_SYSTEM_H
