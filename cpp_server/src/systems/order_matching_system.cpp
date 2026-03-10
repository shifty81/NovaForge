#include "systems/order_matching_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

OrderMatchingSystem::OrderMatchingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OrderMatchingSystem::updateComponent(ecs::Entity& entity,
    components::OrderMatchingState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Remove cancelled orders
    auto removeCancelled = [](std::vector<components::OrderMatchingState::BookOrder>& orders) {
        orders.erase(
            std::remove_if(orders.begin(), orders.end(),
                [](const components::OrderMatchingState::BookOrder& o) {
                    return o.cancelled;
                }),
            orders.end());
    };
    removeCancelled(comp.buy_orders);
    removeCancelled(comp.sell_orders);

    // Sort buy orders: highest price first (best bid)
    std::sort(comp.buy_orders.begin(), comp.buy_orders.end(),
        [](const components::OrderMatchingState::BookOrder& a,
           const components::OrderMatchingState::BookOrder& b) {
            if (a.price != b.price) return a.price > b.price;
            return a.timestamp < b.timestamp; // FIFO
        });

    // Sort sell orders: lowest price first (best ask)
    std::sort(comp.sell_orders.begin(), comp.sell_orders.end(),
        [](const components::OrderMatchingState::BookOrder& a,
           const components::OrderMatchingState::BookOrder& b) {
            if (a.price != b.price) return a.price < b.price;
            return a.timestamp < b.timestamp;
        });

    // Match orders: for each item type, match best buy >= best sell
    bool matched = true;
    while (matched) {
        matched = false;
        for (auto buy_it = comp.buy_orders.begin(); buy_it != comp.buy_orders.end(); ++buy_it) {
            int buy_remaining = buy_it->quantity - buy_it->filled;
            if (buy_remaining <= 0) continue;

            for (auto sell_it = comp.sell_orders.begin(); sell_it != comp.sell_orders.end(); ++sell_it) {
                int sell_remaining = sell_it->quantity - sell_it->filled;
                if (sell_remaining <= 0) continue;
                if (sell_it->item_type != buy_it->item_type) continue;
                if (buy_it->price < sell_it->price) continue;

                // Match!
                int fill_qty = (std::min)(buy_remaining, sell_remaining);
                float trade_price = sell_it->price; // execute at ask
                float trade_value = trade_price * static_cast<float>(fill_qty);

                buy_it->filled += fill_qty;
                sell_it->filled += fill_qty;

                comp.total_matches++;
                comp.total_volume_traded += fill_qty;
                comp.total_value_traded += trade_value;
                comp.fees_collected += trade_value * comp.broker_fee_rate;

                matched = true;
                break;
            }
            if (matched) break;
        }
    }

    // Remove fully filled orders
    auto removeFilled = [](std::vector<components::OrderMatchingState::BookOrder>& orders) {
        orders.erase(
            std::remove_if(orders.begin(), orders.end(),
                [](const components::OrderMatchingState::BookOrder& o) {
                    return o.filled >= o.quantity;
                }),
            orders.end());
    };
    removeFilled(comp.buy_orders);
    removeFilled(comp.sell_orders);
}

bool OrderMatchingSystem::initialize(const std::string& entity_id,
    const std::string& region_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OrderMatchingState>();
    comp->region_id = region_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool OrderMatchingSystem::placeBuyOrder(const std::string& entity_id,
    const std::string& order_id, const std::string& owner_id,
    const std::string& item_type, int quantity, float price) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->buy_orders.size()) >= comp->max_orders_per_side) return false;

    components::OrderMatchingState::BookOrder order;
    order.order_id = order_id;
    order.owner_id = owner_id;
    order.side = components::OrderMatchingState::Side::Buy;
    order.item_type = item_type;
    order.quantity = quantity;
    order.price = price;
    order.timestamp = comp->elapsed;
    comp->buy_orders.push_back(order);
    return true;
}

bool OrderMatchingSystem::placeSellOrder(const std::string& entity_id,
    const std::string& order_id, const std::string& owner_id,
    const std::string& item_type, int quantity, float price) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->sell_orders.size()) >= comp->max_orders_per_side) return false;

    components::OrderMatchingState::BookOrder order;
    order.order_id = order_id;
    order.owner_id = owner_id;
    order.side = components::OrderMatchingState::Side::Sell;
    order.item_type = item_type;
    order.quantity = quantity;
    order.price = price;
    order.timestamp = comp->elapsed;
    comp->sell_orders.push_back(order);
    return true;
}

bool OrderMatchingSystem::cancelOrder(const std::string& entity_id,
    const std::string& order_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& o : comp->buy_orders) {
        if (o.order_id == order_id && !o.cancelled) { o.cancelled = true; return true; }
    }
    for (auto& o : comp->sell_orders) {
        if (o.order_id == order_id && !o.cancelled) { o.cancelled = true; return true; }
    }
    return false;
}

bool OrderMatchingSystem::setBrokerFeeRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->broker_fee_rate = rate;
    return true;
}

int OrderMatchingSystem::getBuyOrderCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->buy_orders.size()) : 0;
}

int OrderMatchingSystem::getSellOrderCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->sell_orders.size()) : 0;
}

int OrderMatchingSystem::getTotalMatches(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_matches : 0;
}

int OrderMatchingSystem::getTotalVolumeTraded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_volume_traded : 0;
}

float OrderMatchingSystem::getTotalValueTraded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_value_traded : 0.0f;
}

float OrderMatchingSystem::getFeesCollected(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fees_collected : 0.0f;
}

float OrderMatchingSystem::getBrokerFeeRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->broker_fee_rate : 0.0f;
}

} // namespace systems
} // namespace atlas
