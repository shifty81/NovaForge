#ifndef NOVAFORGE_SYSTEMS_SHIP_SKIN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_SKIN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship skin (SKIN) collection and equip management
 *
 * Manages a player's collection of ship skins.  Skins have five rarity
 * tiers (Common → Legendary) and an optional ship_type restriction.
 * Only one skin may be equipped at a time; equipping a new skin
 * automatically unequips the previous one.  The collection is capped
 * at max_skins.
 */
class ShipSkinSystem
    : public ecs::SingleComponentSystem<components::ShipSkinCollection> {
public:
    explicit ShipSkinSystem(ecs::World* world);
    ~ShipSkinSystem() override = default;

    std::string getName() const override { return "ShipSkinSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& owner_id);

    // --- Skin management ---
    bool addSkin(const std::string& entity_id,
                 const std::string& skin_id,
                 const std::string& name,
                 const std::string& ship_type,
                 components::ShipSkinCollection::Rarity rarity,
                 const std::string& color_primary,
                 const std::string& color_secondary);
    bool removeSkin(const std::string& entity_id,
                    const std::string& skin_id);
    bool equipSkin(const std::string& entity_id,
                   const std::string& skin_id);
    bool unequipSkin(const std::string& entity_id);

    // --- Queries ---
    int  getSkinCount(const std::string& entity_id) const;
    std::string getEquippedSkinId(const std::string& entity_id) const;
    int  getSkinCountByRarity(const std::string& entity_id,
                              components::ShipSkinCollection::Rarity rarity) const;
    int  getTotalAcquired(const std::string& entity_id) const;
    bool hasSkin(const std::string& entity_id,
                 const std::string& skin_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ShipSkinCollection& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_SKIN_SYSTEM_H
