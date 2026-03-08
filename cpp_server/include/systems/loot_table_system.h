#ifndef NOVAFORGE_SYSTEMS_LOOT_TABLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOOT_TABLE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>
#include <vector>
#include <utility>

namespace atlas {
namespace systems {

/**
 * @brief Configurable weighted loot tables for encounters, missions, and exploration
 *
 * Manages named loot tables with weighted entries.  Each entry has a rarity
 * tier and quantity range.  Rolls are resolved deterministically (seeded)
 * so replay/testing is reproducible.  Supports luck modifier that boosts
 * rare-tier weights and per-entry rarity filtering.
 */
class LootTableSystem : public ecs::SingleComponentSystem<components::LootTableState> {
public:
    explicit LootTableSystem(ecs::World* world);
    ~LootTableSystem() override = default;

    std::string getName() const override { return "LootTableSystem"; }

    bool initialize(const std::string& entity_id, const std::string& table_id);
    bool addEntry(const std::string& entity_id, const std::string& item_id,
                  const std::string& rarity, float weight, int min_qty, int max_qty);
    bool removeEntry(const std::string& entity_id, const std::string& item_id);
    bool setLuckModifier(const std::string& entity_id, float luck);

    /**
     * @brief Roll the loot table once using a deterministic seed value.
     * Returns the chosen item_id (empty string on failure).
     */
    std::string rollLoot(const std::string& entity_id, float seed_value);

    /**
     * @brief Get all entries matching a rarity tier.
     */
    std::vector<std::string> getEntriesByRarity(const std::string& entity_id,
                                                 const std::string& rarity) const;

    int   getEntryCount(const std::string& entity_id) const;
    float getTotalWeight(const std::string& entity_id) const;
    int   getTotalRolls(const std::string& entity_id) const;
    int   getTotalDrops(const std::string& entity_id) const;
    float getLuckModifier(const std::string& entity_id) const;
    std::string getTableId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::LootTableState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOOT_TABLE_SYSTEM_H
