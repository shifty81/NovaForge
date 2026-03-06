#ifndef NOVAFORGE_SYSTEMS_MARKET_BROWSER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MARKET_BROWSER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Provides market order browsing, searching, and filtering for trade hubs
 *
 * Allows players to query buy/sell orders at stations, filter by item type or
 * price range, sort results, and execute buy/sell transactions. Tracks recent
 * searches and favorite items for quick access. Supports the vertical-slice
 * trade loop: dock → browse market → buy/sell → undock.
 */
class MarketBrowserSystem : public ecs::SingleComponentSystem<components::MarketBrowserState> {
public:
    explicit MarketBrowserSystem(ecs::World* world);
    ~MarketBrowserSystem() override = default;

    std::string getName() const override { return "MarketBrowserSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Order management (station populates these)
    bool addOrder(const std::string& entity_id, const std::string& order_id,
                  const std::string& item_name, bool is_buy_order,
                  double price, int quantity);
    int getOrderCount(const std::string& entity_id) const;
    bool removeOrder(const std::string& entity_id, const std::string& order_id);
    bool hasOrder(const std::string& entity_id, const std::string& order_id) const;

    // Queries
    int getBuyOrderCount(const std::string& entity_id) const;
    int getSellOrderCount(const std::string& entity_id) const;
    double getLowestSellPrice(const std::string& entity_id, const std::string& item_name) const;
    double getHighestBuyPrice(const std::string& entity_id, const std::string& item_name) const;
    int getOrderCountForItem(const std::string& entity_id, const std::string& item_name) const;

    // Filtering
    bool setFilter(const std::string& entity_id, const std::string& item_filter);
    std::string getFilter(const std::string& entity_id) const;
    int getFilteredCount(const std::string& entity_id) const;

    // Favorites
    bool addFavorite(const std::string& entity_id, const std::string& item_name);
    bool removeFavorite(const std::string& entity_id, const std::string& item_name);
    bool isFavorite(const std::string& entity_id, const std::string& item_name) const;
    int getFavoriteCount(const std::string& entity_id) const;

    // Transaction history
    bool recordTransaction(const std::string& entity_id, const std::string& item_name,
                           bool is_buy, double price, int quantity);
    int getTransactionCount(const std::string& entity_id) const;
    double getTotalSpent(const std::string& entity_id) const;
    double getTotalEarned(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MarketBrowserState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MARKET_BROWSER_SYSTEM_H
