#ifndef NOVAFORGE_SYSTEMS_PLAYER_HANGAR_INVENTORY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_HANGAR_INVENTORY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Per-player per-station hangar item storage
 *
 * Manages items stored in station hangars on a per-player basis.
 * Handles deposits, withdrawals, capacity enforcement, stacking
 * of identical items, and value tracking.  Essential for the
 * dock → store → equip → undock gameplay loop.
 */
class PlayerHangarInventorySystem : public ecs::SingleComponentSystem<components::PlayerHangarInventory> {
public:
    explicit PlayerHangarInventorySystem(ecs::World* world);
    ~PlayerHangarInventorySystem() override = default;

    std::string getName() const override { return "PlayerHangarInventorySystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, const std::string& player_id,
                    const std::string& station_id, double max_volume);

    // Item operations
    bool depositItem(const std::string& entity_id, const std::string& item_id,
                     const std::string& item_name, const std::string& item_type,
                     int quantity, double volume_per_unit, double estimated_value);
    bool withdrawItem(const std::string& entity_id, const std::string& item_id, int quantity);
    bool transferItem(const std::string& source_entity, const std::string& dest_entity,
                      const std::string& item_id, int quantity);

    // Capacity
    bool setMaxVolume(const std::string& entity_id, double max_volume);

    // Queries
    int getItemCount(const std::string& entity_id) const;
    int getItemQuantity(const std::string& entity_id, const std::string& item_id) const;
    double getUsedVolume(const std::string& entity_id) const;
    double getRemainingVolume(const std::string& entity_id) const;
    double getTotalValueStored(const std::string& entity_id) const;
    int getTotalDeposits(const std::string& entity_id) const;
    int getTotalWithdrawals(const std::string& entity_id) const;
    std::string getStationId(const std::string& entity_id) const;
    std::string getPlayerId(const std::string& entity_id) const;
    bool hasItem(const std::string& entity_id, const std::string& item_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerHangarInventory& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_HANGAR_INVENTORY_SYSTEM_H
