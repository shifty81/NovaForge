#ifndef NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Dynamic incursion spawning and fleet coordination system
 *
 * Manages constellation-wide incursion encounters.  Sites are spawned at
 * different difficulty tiers (Vanguard, Assault, Headquarters) and
 * completing them reduces the incursion's influence level.  When influence
 * reaches zero the incursion withdraws.  Fleet coordination is rewarded
 * with loyalty point payouts scaled by fleet size and site tier.
 */
class IncursionSystem : public ecs::SingleComponentSystem<components::IncursionState> {
public:
    explicit IncursionSystem(ecs::World* world);
    ~IncursionSystem() override = default;

    std::string getName() const override { return "IncursionSystem"; }

    // --- public API: mutators ---
    bool initialize(const std::string& entity_id,
                    const std::string& constellation_id = "");
    bool spawnSite(const std::string& entity_id, const std::string& site_id,
                   components::IncursionState::Tier tier =
                       components::IncursionState::Tier::Vanguard);
    bool completeSite(const std::string& entity_id, const std::string& site_id);
    bool registerFleetMember(const std::string& entity_id,
                             const std::string& site_id,
                             const std::string& pilot_id);
    bool applyInfluenceDelta(const std::string& entity_id, float delta);

    // --- public API: queries ---
    int   getSiteCount(const std::string& entity_id) const;
    float getInfluence(const std::string& entity_id) const;
    bool  isWithdrawn(const std::string& entity_id) const;
    int   getCompletedSiteCount(const std::string& entity_id) const;
    float getTotalLPPaid(const std::string& entity_id) const;
    int   getFleetSize(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::IncursionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H
