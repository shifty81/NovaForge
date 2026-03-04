#ifndef NOVAFORGE_SYSTEMS_LOYALTY_POINT_STORE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LOYALTY_POINT_STORE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Loyalty point store system for faction LP rewards and purchases
 *
 * Manages LP earning from missions, store inventory, and purchasing
 * unique faction items with LP and ISK costs.
 */
class LoyaltyPointStoreSystem : public ecs::SingleComponentSystem<components::LoyaltyPointStore> {
public:
    explicit LoyaltyPointStoreSystem(ecs::World* world);
    ~LoyaltyPointStoreSystem() override = default;

    std::string getName() const override { return "LoyaltyPointStoreSystem"; }

    bool initialize(const std::string& entity_id, const std::string& store_id,
                    const std::string& faction_id);
    bool addItem(const std::string& entity_id, const std::string& item_id,
                 const std::string& name, const std::string& category,
                 int lp_cost, float isk_cost, int tier);
    bool removeItem(const std::string& entity_id, const std::string& item_id);
    bool registerPlayer(const std::string& entity_id, const std::string& player_id);
    bool earnLP(const std::string& entity_id, const std::string& player_id, int amount);
    bool purchaseItem(const std::string& entity_id, const std::string& player_id,
                      const std::string& item_id);
    int getBalance(const std::string& entity_id, const std::string& player_id) const;
    int getItemCount(const std::string& entity_id) const;
    int getPlayerCount(const std::string& entity_id) const;
    int getTotalPurchases(const std::string& entity_id) const;
    float getTotalISKCollected(const std::string& entity_id) const;
    int getItemsByCategory(const std::string& entity_id, const std::string& category) const;

protected:
    void updateComponent(ecs::Entity& entity, components::LoyaltyPointStore& store, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LOYALTY_POINT_STORE_SYSTEM_H
