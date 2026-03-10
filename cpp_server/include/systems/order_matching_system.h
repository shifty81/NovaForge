#ifndef NOVAFORGE_SYSTEMS_ORDER_MATCHING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ORDER_MATCHING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Market order matching engine
 *
 * Maintains a price-time-priority order book for a market region.
 * Each tick, the system matches the best buy against the best sell
 * for each item type.  Partial fills are supported.  Broker fees
 * are deducted on each trade.
 */
class OrderMatchingSystem : public ecs::SingleComponentSystem<components::OrderMatchingState> {
public:
    explicit OrderMatchingSystem(ecs::World* world);
    ~OrderMatchingSystem() override = default;

    std::string getName() const override { return "OrderMatchingSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& region_id = "");
    bool placeBuyOrder(const std::string& entity_id, const std::string& order_id,
                       const std::string& owner_id, const std::string& item_type,
                       int quantity, float price);
    bool placeSellOrder(const std::string& entity_id, const std::string& order_id,
                        const std::string& owner_id, const std::string& item_type,
                        int quantity, float price);
    bool cancelOrder(const std::string& entity_id, const std::string& order_id);
    bool setBrokerFeeRate(const std::string& entity_id, float rate);

    int  getBuyOrderCount(const std::string& entity_id) const;
    int  getSellOrderCount(const std::string& entity_id) const;
    int  getTotalMatches(const std::string& entity_id) const;
    int  getTotalVolumeTraded(const std::string& entity_id) const;
    float getTotalValueTraded(const std::string& entity_id) const;
    float getFeesCollected(const std::string& entity_id) const;
    float getBrokerFeeRate(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OrderMatchingState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ORDER_MATCHING_SYSTEM_H
