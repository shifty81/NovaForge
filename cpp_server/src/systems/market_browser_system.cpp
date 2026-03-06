#include "systems/market_browser_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using MBS = components::MarketBrowserState;
}

MarketBrowserSystem::MarketBrowserSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MarketBrowserSystem::updateComponent(ecs::Entity& entity,
    components::MarketBrowserState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool MarketBrowserSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MarketBrowserState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool MarketBrowserSystem::addOrder(const std::string& entity_id,
    const std::string& order_id, const std::string& item_name,
    bool is_buy_order, double price, int quantity) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Duplicate check
    for (const auto& o : state->orders) {
        if (o.order_id == order_id) return false;
    }
    if (static_cast<int>(state->orders.size()) >= state->max_orders) return false;
    MBS::MarketOrder order;
    order.order_id = order_id;
    order.item_name = item_name;
    order.is_buy_order = is_buy_order;
    order.price = price;
    order.quantity = quantity;
    state->orders.push_back(order);
    return true;
}

int MarketBrowserSystem::getOrderCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->orders.size()) : 0;
}

bool MarketBrowserSystem::removeOrder(const std::string& entity_id,
    const std::string& order_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->orders.begin(), state->orders.end(),
        [&](const MBS::MarketOrder& o) { return o.order_id == order_id; });
    if (it == state->orders.end()) return false;
    state->orders.erase(it);
    return true;
}

bool MarketBrowserSystem::hasOrder(const std::string& entity_id,
    const std::string& order_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& o : state->orders) {
        if (o.order_id == order_id) return true;
    }
    return false;
}

int MarketBrowserSystem::getBuyOrderCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& o : state->orders) {
        if (o.is_buy_order) count++;
    }
    return count;
}

int MarketBrowserSystem::getSellOrderCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& o : state->orders) {
        if (!o.is_buy_order) count++;
    }
    return count;
}

double MarketBrowserSystem::getLowestSellPrice(const std::string& entity_id,
    const std::string& item_name) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double lowest = -1.0;
    for (const auto& o : state->orders) {
        if (!o.is_buy_order && o.item_name == item_name) {
            if (lowest < 0.0 || o.price < lowest) lowest = o.price;
        }
    }
    return lowest < 0.0 ? 0.0 : lowest;
}

double MarketBrowserSystem::getHighestBuyPrice(const std::string& entity_id,
    const std::string& item_name) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double highest = 0.0;
    for (const auto& o : state->orders) {
        if (o.is_buy_order && o.item_name == item_name) {
            if (o.price > highest) highest = o.price;
        }
    }
    return highest;
}

int MarketBrowserSystem::getOrderCountForItem(const std::string& entity_id,
    const std::string& item_name) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& o : state->orders) {
        if (o.item_name == item_name) count++;
    }
    return count;
}

bool MarketBrowserSystem::setFilter(const std::string& entity_id,
    const std::string& item_filter) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->current_filter = item_filter;
    return true;
}

std::string MarketBrowserSystem::getFilter(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->current_filter : "";
}

int MarketBrowserSystem::getFilteredCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    if (state->current_filter.empty()) return static_cast<int>(state->orders.size());
    int count = 0;
    for (const auto& o : state->orders) {
        if (o.item_name.find(state->current_filter) != std::string::npos) count++;
    }
    return count;
}

bool MarketBrowserSystem::addFavorite(const std::string& entity_id,
    const std::string& item_name) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& f : state->favorites) {
        if (f == item_name) return false;
    }
    if (static_cast<int>(state->favorites.size()) >= state->max_favorites) return false;
    state->favorites.push_back(item_name);
    return true;
}

bool MarketBrowserSystem::removeFavorite(const std::string& entity_id,
    const std::string& item_name) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find(state->favorites.begin(), state->favorites.end(), item_name);
    if (it == state->favorites.end()) return false;
    state->favorites.erase(it);
    return true;
}

bool MarketBrowserSystem::isFavorite(const std::string& entity_id,
    const std::string& item_name) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& f : state->favorites) {
        if (f == item_name) return true;
    }
    return false;
}

int MarketBrowserSystem::getFavoriteCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->favorites.size()) : 0;
}

bool MarketBrowserSystem::recordTransaction(const std::string& entity_id,
    const std::string& item_name, bool is_buy, double price, int quantity) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->transactions.size()) >= state->max_transactions) {
        // Evict oldest transaction
        state->transactions.erase(state->transactions.begin());
    }
    MBS::Transaction tx;
    tx.item_name = item_name;
    tx.is_buy = is_buy;
    tx.price = price;
    tx.quantity = quantity;
    tx.timestamp = state->elapsed_time;
    state->transactions.push_back(tx);
    double total = price * quantity;
    if (is_buy) {
        state->total_spent += total;
    } else {
        state->total_earned += total;
    }
    return true;
}

int MarketBrowserSystem::getTransactionCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->transactions.size()) : 0;
}

double MarketBrowserSystem::getTotalSpent(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_spent : 0.0;
}

double MarketBrowserSystem::getTotalEarned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_earned : 0.0;
}

} // namespace systems
} // namespace atlas
