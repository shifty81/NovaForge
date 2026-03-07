#ifndef NOVAFORGE_SYSTEMS_COMBAT_LOOT_DROP_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_LOOT_DROP_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Combat loot drop management — configure drop tables, spawn loot from kills
 *
 * When a ship or NPC is destroyed, this system spawns loot entities based on
 * a configurable drop table.  Each entry in the table has an item ID, a
 * drop chance (0.0-1.0), a min/max quantity range, and a rarity tier.
 * The system records pending drops which are resolved each tick using a
 * deterministic roll.  Drop results are logged for combat after-action.
 */
class CombatLootDropSystem : public ecs::SingleComponentSystem<components::CombatLootDrop> {
public:
    explicit CombatLootDropSystem(ecs::World* world);
    ~CombatLootDropSystem() override = default;

    std::string getName() const override { return "CombatLootDropSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addDropEntry(const std::string& entity_id, const std::string& item_id,
                      float drop_chance, int min_qty, int max_qty,
                      const std::string& rarity);
    bool removeDropEntry(const std::string& entity_id, const std::string& item_id);
    bool triggerDrop(const std::string& entity_id, const std::string& source_id);

    int getDropEntryCount(const std::string& entity_id) const;
    int getTotalDropsTriggered(const std::string& entity_id) const;
    int getTotalItemsDropped(const std::string& entity_id) const;
    int getPendingDropCount(const std::string& entity_id) const;
    std::string getLastDropSource(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CombatLootDrop& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_LOOT_DROP_SYSTEM_H
