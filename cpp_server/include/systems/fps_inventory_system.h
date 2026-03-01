#ifndef NOVAFORGE_SYSTEMS_FPS_INVENTORY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_INVENTORY_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Personal inventory system for FPS characters
 *
 * Manages items carried on foot — weapons, tools, consumables.  Ties into
 * FPSCombatSystem (equipped weapon), FPSHandSystem (tool attachment), and
 * SurvivalSystem (consumable use such as oxygen refills and food).
 */
class FPSInventorySystem : public ecs::System {
public:
    explicit FPSInventorySystem(ecs::World* world);
    ~FPSInventorySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FPSInventorySystem"; }

    // --- Inventory management ---

    /** Create an FPSInventoryComponent for a player */
    bool createInventory(const std::string& player_id, int max_slots = 8);

    /** Add an item to the player's inventory */
    bool addItem(const std::string& player_id,
                 const std::string& item_id,
                 const std::string& item_name,
                 int quantity = 1);

    /** Remove an item from the player's inventory */
    bool removeItem(const std::string& player_id,
                    const std::string& item_id);

    /** Check if a player has a specific item */
    bool hasItem(const std::string& player_id,
                 const std::string& item_id) const;

    // --- Equipment ---

    /** Equip a weapon from inventory */
    bool equipWeapon(const std::string& player_id,
                     const std::string& weapon_item_id);

    /** Unequip the current weapon */
    bool unequipWeapon(const std::string& player_id);

    /** Equip a tool from inventory */
    bool equipTool(const std::string& player_id,
                   const std::string& tool_item_id);

    /** Unequip the current tool */
    bool unequipTool(const std::string& player_id);

    // --- Consumables (ties into SurvivalSystem) ---

    /** Use a consumable item (oxygen refill, food, stimpack, etc.) */
    bool useConsumable(const std::string& player_id,
                       const std::string& item_id);

    // --- Queries ---

    int getItemCount(const std::string& player_id) const;
    int getMaxSlots(const std::string& player_id) const;
    bool isInventoryFull(const std::string& player_id) const;
    std::string getEquippedWeapon(const std::string& player_id) const;
    std::string getEquippedTool(const std::string& player_id) const;

private:
    static constexpr const char* INV_PREFIX = "fps_inv_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_INVENTORY_SYSTEM_H
