#include "systems/market_trade_execution_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MarketTradeExecutionSystem::MarketTradeExecutionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MarketTradeExecutionSystem::updateComponent(ecs::Entity& entity,
    components::MarketTradeState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Process queued orders each tick
    auto it = comp.queued_orders.begin();
    while (it != comp.queued_orders.end()) {
        auto& order = *it;
        bool completed = false;

        if (order.order_type == "buy") {
            double total_cost = order.price_per_unit * order.quantity;
            double broker_fee = total_cost * comp.broker_fee_rate;
            double total_with_fee = total_cost + broker_fee;

            if (order.available_balance >= total_with_fee) {
                // Full fill
                order.filled_quantity = order.quantity;
                comp.total_broker_fees += broker_fee;
                comp.total_volume_traded += total_cost;
                comp.completed_trades++;
                completed = true;
            } else if (order.available_balance > broker_fee) {
                // Partial fill — buy as much as balance allows
                double usable = order.available_balance - broker_fee;
                int affordable = static_cast<int>(usable / order.price_per_unit);
                if (affordable > 0) {
                    order.filled_quantity = affordable;
                    double partial_cost = order.price_per_unit * affordable;
                    double partial_fee = partial_cost * comp.broker_fee_rate;
                    comp.total_broker_fees += partial_fee;
                    comp.total_volume_traded += partial_cost;
                    comp.completed_trades++;
                    comp.partial_fills++;
                    completed = true;
                } else {
                    completed = true; // Can't afford anything, discard
                }
            } else {
                completed = true; // Can't even cover fee, discard
            }
        } else if (order.order_type == "sell") {
            if (order.available_stock >= order.quantity) {
                // Full fill
                double revenue = order.price_per_unit * order.quantity;
                double broker_fee = revenue * comp.broker_fee_rate;
                order.filled_quantity = order.quantity;
                comp.total_broker_fees += broker_fee;
                comp.total_volume_traded += revenue;
                comp.completed_trades++;
                completed = true;
            } else if (order.available_stock > 0) {
                // Partial fill — sell only what's available
                double revenue = order.price_per_unit * order.available_stock;
                double broker_fee = revenue * comp.broker_fee_rate;
                order.filled_quantity = order.available_stock;
                comp.total_broker_fees += broker_fee;
                comp.total_volume_traded += revenue;
                comp.completed_trades++;
                comp.partial_fills++;
                completed = true;
            } else {
                completed = true; // No stock, discard
            }
        } else {
            completed = true; // Unknown order type, discard
        }

        if (completed) {
            it = comp.queued_orders.erase(it);
        } else {
            ++it;
        }
    }
}

bool MarketTradeExecutionSystem::initialize(const std::string& entity_id,
    double broker_fee_rate) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MarketTradeState>();
    comp->broker_fee_rate = std::max(0.0, std::min(1.0, broker_fee_rate));
    entity->addComponent(std::move(comp));
    return true;
}

bool MarketTradeExecutionSystem::queueBuyOrder(const std::string& entity_id,
    const std::string& item_id, int quantity, double price_per_unit,
    double buyer_balance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (quantity <= 0 || price_per_unit <= 0.0 || buyer_balance < 0.0) return false;
    if (static_cast<int>(comp->queued_orders.size()) >= comp->max_queued_orders) return false;

    components::MarketTradeState::TradeOrder order;
    order.order_type = "buy";
    order.item_id = item_id;
    order.quantity = quantity;
    order.price_per_unit = price_per_unit;
    order.available_balance = buyer_balance;
    comp->queued_orders.push_back(order);
    return true;
}

bool MarketTradeExecutionSystem::queueSellOrder(const std::string& entity_id,
    const std::string& item_id, int quantity, double price_per_unit,
    int seller_stock) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (quantity <= 0 || price_per_unit <= 0.0 || seller_stock < 0) return false;
    if (static_cast<int>(comp->queued_orders.size()) >= comp->max_queued_orders) return false;

    components::MarketTradeState::TradeOrder order;
    order.order_type = "sell";
    order.item_id = item_id;
    order.quantity = quantity;
    order.price_per_unit = price_per_unit;
    order.available_stock = seller_stock;
    comp->queued_orders.push_back(order);
    return true;
}

bool MarketTradeExecutionSystem::setBrokerFeeRate(const std::string& entity_id,
    double rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->broker_fee_rate = std::max(0.0, std::min(1.0, rate));
    return true;
}

int MarketTradeExecutionSystem::getQueuedOrderCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->queued_orders.size()) : 0;
}

int MarketTradeExecutionSystem::getCompletedTradeCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->completed_trades : 0;
}

double MarketTradeExecutionSystem::getTotalBrokerFees(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_broker_fees : 0.0;
}

double MarketTradeExecutionSystem::getTotalVolumeTraded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_volume_traded : 0.0;
}

int MarketTradeExecutionSystem::getPartialFillCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->partial_fills : 0;
}

} // namespace systems
} // namespace atlas
