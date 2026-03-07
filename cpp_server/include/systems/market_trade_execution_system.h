#ifndef NOVAFORGE_SYSTEMS_MARKET_TRADE_EXECUTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MARKET_TRADE_EXECUTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Market trade execution — buy, sell, and broker fee calculation
 *
 * Processes trade orders queued on a market entity.  Each tick the system
 * evaluates queued orders, validates ISC balance (buy) or item quantity
 * (sell), applies a configurable broker fee, and records the completed
 * trade.  Partial fills are supported — if a buy order exceeds available
 * stock, only the available quantity is purchased.
 */
class MarketTradeExecutionSystem : public ecs::SingleComponentSystem<components::MarketTradeState> {
public:
    explicit MarketTradeExecutionSystem(ecs::World* world);
    ~MarketTradeExecutionSystem() override = default;

    std::string getName() const override { return "MarketTradeExecutionSystem"; }

public:
    bool initialize(const std::string& entity_id, double broker_fee_rate);
    bool queueBuyOrder(const std::string& entity_id, const std::string& item_id,
                       int quantity, double price_per_unit, double buyer_balance);
    bool queueSellOrder(const std::string& entity_id, const std::string& item_id,
                        int quantity, double price_per_unit, int seller_stock);
    bool setBrokerFeeRate(const std::string& entity_id, double rate);

    int getQueuedOrderCount(const std::string& entity_id) const;
    int getCompletedTradeCount(const std::string& entity_id) const;
    double getTotalBrokerFees(const std::string& entity_id) const;
    double getTotalVolumeTraded(const std::string& entity_id) const;
    int getPartialFillCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::MarketTradeState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MARKET_TRADE_EXECUTION_SYSTEM_H
