#ifndef NOVAFORGE_SYSTEMS_ASSET_SAFETY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ASSET_SAFETY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief EVE-style asset safety — structure-destruction asset relocation.
 *
 * When a player-owned structure is destroyed or unanchored, any assets
 * inside enter an "asset safety wrap" held at a nearby NPC station.  The
 * owner has safety_duration seconds (default 14 days) to claim them; after
 * that the entries expire and the items are lost.  Multiple structures can
 * generate entries for the same owner entity.
 */
class AssetSafetySystem
    : public ecs::SingleComponentSystem<components::AssetSafetyState> {
public:
    explicit AssetSafetySystem(ecs::World* world);
    ~AssetSafetySystem() override = default;

    std::string getName() const override { return "AssetSafetySystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_id);

    // --- Safety entry management ---
    bool triggerSafety(const std::string& entity_id,
                       const std::string& structure_id,
                       const std::string& structure_name,
                       const std::string& asset_id,
                       const std::string& asset_name,
                       int quantity);
    bool claimAsset(const std::string& entity_id,
                    const std::string& asset_id);
    bool claimAll(const std::string& entity_id);

    // --- Queries ---
    int         getEntryCount(const std::string& entity_id) const;
    int         getUnclaimedCount(const std::string& entity_id) const;
    int         getExpiredCount(const std::string& entity_id) const;
    int         getTotalTriggered(const std::string& entity_id) const;
    int         getTotalClaimed(const std::string& entity_id) const;
    bool        hasEntry(const std::string& entity_id,
                         const std::string& asset_id) const;
    std::string getOwner(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AssetSafetyState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ASSET_SAFETY_SYSTEM_H
