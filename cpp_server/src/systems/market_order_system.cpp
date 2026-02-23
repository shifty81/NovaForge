#include "systems/market_order_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

MarketOrderSystem::MarketOrderSystem(ecs::World* world)
    : System(world) {
}

void MarketOrderSystem::update(float delta_time) {
    // Update market orders
    auto order_entities = world_->getEntities<components::MarketOrder>();
    for (auto* entity : order_entities) {
        auto* order = entity->getComponent<components::MarketOrder>();
        if (!order || order->is_filled) continue;
        order->elapsed_time += delta_time;
    }

    // Update AI fleet dispatches
    auto dispatch_entities = world_->getEntities<components::AIFleetDispatch>();
    for (auto* entity : dispatch_entities) {
        auto* dispatch = entity->getComponent<components::AIFleetDispatch>();
        if (!dispatch || !dispatch->dispatched) continue;
        dispatch->elapsed += delta_time;
    }
}

void MarketOrderSystem::placeOrder(const std::string& entity_id,
                                   components::MarketOrder::OrderType order_type,
                                   const std::string& item, int qty, float price,
                                   const std::string& region, const std::string& station,
                                   const std::string& owner) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* order = entity->getComponent<components::MarketOrder>();
    if (!order) return;

    order->type = order_type;
    order->item_type = item;
    order->quantity = qty;
    order->quantity_remaining = qty;
    order->price_per_unit = price;
    order->region_id = region;
    order->station_id = station;
    order->owner_id = owner;
    order->elapsed_time = 0.0f;
    order->is_filled = false;
}

bool MarketOrderSystem::cancelOrder(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* order = entity->getComponent<components::MarketOrder>();
    if (!order) return false;
    order->is_filled = true;
    order->quantity_remaining = 0;
    return true;
}

int MarketOrderSystem::fillOrder(const std::string& entity_id, int fill_qty) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* order = entity->getComponent<components::MarketOrder>();
    if (!order || order->is_filled) return 0;

    int actual = std::min(fill_qty, order->quantity_remaining);
    order->quantity_remaining -= actual;
    if (order->quantity_remaining <= 0) {
        order->is_filled = true;
    }
    return actual;
}

std::vector<std::string> MarketOrderSystem::getOrdersForRegion(const std::string& region_id) const {
    std::vector<std::string> result;
    auto order_entities = world_->getEntities<components::MarketOrder>();
    for (auto* entity : order_entities) {
        auto* order = entity->getComponent<components::MarketOrder>();
        if (!order || order->is_filled) continue;
        if (order->region_id == region_id) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

std::string MarketOrderSystem::dispatchAIFleet(const std::string& order_entity_id,
                                                components::AIFleetDispatch::DispatchType dispatch_type,
                                                const std::string& target_system, int fleet_size) {
    std::string dispatch_id = order_entity_id + "_dispatch";
    auto* dispatch_entity = world_->createEntity(dispatch_id);
    if (!dispatch_entity) return "";

    auto dispatch_comp = std::make_unique<components::AIFleetDispatch>();
    dispatch_comp->type = dispatch_type;
    dispatch_comp->order_id = order_entity_id;
    dispatch_comp->target_system_id = target_system;
    dispatch_comp->fleet_size = fleet_size;
    dispatch_comp->dispatched = true;
    dispatch_comp->estimated_completion = 60.0f * fleet_size;
    dispatch_entity->addComponent(std::move(dispatch_comp));

    return dispatch_id;
}

std::vector<std::string> MarketOrderSystem::getActiveDispatches() const {
    std::vector<std::string> result;
    auto dispatch_entities = world_->getEntities<components::AIFleetDispatch>();
    for (auto* entity : dispatch_entities) {
        auto* dispatch = entity->getComponent<components::AIFleetDispatch>();
        if (!dispatch) continue;
        if (dispatch->dispatched && !dispatch->isComplete()) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

bool MarketOrderSystem::isOrderExpired(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* order = entity->getComponent<components::MarketOrder>();
    if (!order) return false;
    return order->elapsed_time >= order->expiry_time;
}

} // namespace systems
} // namespace atlas
