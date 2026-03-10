#ifndef NOVAFORGE_SYSTEMS_MARKET_API_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MARKET_API_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Client-facing market data API system
 *
 * Provides game clients with access to order-book snapshots, price
 * history, and subscription feeds.  Clients subscribe to item types
 * and receive periodic price pushes driven by the server tick.
 * Handles incoming requests (buy_orders, sell_orders, history) and
 * marks them fulfilled after population.
 */
class MarketApiSystem
    : public ecs::SingleComponentSystem<components::MarketApiState> {
public:
    explicit MarketApiSystem(ecs::World* world);
    ~MarketApiSystem() override = default;

    std::string getName() const override { return "MarketApiSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& region_id = "");

    // --- Subscriptions ---
    bool subscribe(const std::string& entity_id,
                   const std::string& client_id,
                   const std::string& item_type);
    bool unsubscribe(const std::string& entity_id,
                     const std::string& client_id,
                     const std::string& item_type);
    bool isSubscribed(const std::string& entity_id,
                      const std::string& client_id,
                      const std::string& item_type) const;
    int  getSubscriptionCount(const std::string& entity_id) const;

    // --- Client requests ---
    bool submitRequest(const std::string& entity_id,
                       const std::string& client_id,
                       const std::string& request_type,
                       const std::string& item_type);
    int  getPendingRequestCount(const std::string& entity_id) const;
    int  getTotalRequests(const std::string& entity_id) const;

    // --- Price history ---
    bool recordSnapshot(const std::string& entity_id,
                        float best_buy, float best_sell, float volume);
    int  getHistoryCount(const std::string& entity_id) const;
    int  getTotalPushes(const std::string& entity_id) const;

    // --- Config ---
    bool setPushInterval(const std::string& entity_id, float interval);
    float getPushInterval(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MarketApiState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MARKET_API_SYSTEM_H
