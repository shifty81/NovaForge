#include "systems/loyalty_point_store_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using LPS = components::LoyaltyPointStore;
using StoreItem = components::LoyaltyPointStore::StoreItem;
using LPBalance = components::LoyaltyPointStore::LPBalance;

StoreItem* findItem(LPS* store, const std::string& item_id) {
    for (auto& i : store->items) {
        if (i.item_id == item_id) return &i;
    }
    return nullptr;
}

const StoreItem* findItemConst(const LPS* store, const std::string& item_id) {
    for (const auto& i : store->items) {
        if (i.item_id == item_id) return &i;
    }
    return nullptr;
}

LPBalance* findBalance(LPS* store, const std::string& player_id) {
    for (auto& b : store->balances) {
        if (b.player_id == player_id) return &b;
    }
    return nullptr;
}

const LPBalance* findBalanceConst(const LPS* store, const std::string& player_id) {
    for (const auto& b : store->balances) {
        if (b.player_id == player_id) return &b;
    }
    return nullptr;
}

} // anonymous namespace

LoyaltyPointStoreSystem::LoyaltyPointStoreSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void LoyaltyPointStoreSystem::updateComponent(ecs::Entity& /*entity*/, components::LoyaltyPointStore& /*store*/, float /*delta_time*/) {
    // No per-tick logic needed
}

bool LoyaltyPointStoreSystem::initialize(const std::string& entity_id,
    const std::string& store_id, const std::string& faction_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::LoyaltyPointStore>();
    comp->store_id = store_id;
    comp->faction_id = faction_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool LoyaltyPointStoreSystem::addItem(const std::string& entity_id,
    const std::string& item_id, const std::string& name, const std::string& category,
    int lp_cost, float isk_cost, int tier) {
    auto* store = getComponentFor(entity_id);
    if (!store) return false;
    if (static_cast<int>(store->items.size()) >= store->max_items) return false;
    if (findItem(store, item_id)) return false;

    StoreItem item;
    item.item_id = item_id;
    item.name = name;
    item.category = category;
    item.lp_cost = lp_cost;
    item.isk_cost = isk_cost;
    item.tier = tier;
    store->items.push_back(item);
    return true;
}

bool LoyaltyPointStoreSystem::removeItem(const std::string& entity_id,
    const std::string& item_id) {
    auto* store = getComponentFor(entity_id);
    if (!store) return false;

    auto it = std::remove_if(store->items.begin(), store->items.end(),
        [&](const StoreItem& i) { return i.item_id == item_id; });
    if (it == store->items.end()) return false;
    store->items.erase(it, store->items.end());
    return true;
}

bool LoyaltyPointStoreSystem::registerPlayer(const std::string& entity_id,
    const std::string& player_id) {
    auto* store = getComponentFor(entity_id);
    if (!store) return false;
    if (static_cast<int>(store->balances.size()) >= store->max_players) return false;
    if (findBalance(store, player_id)) return false;

    LPBalance bal;
    bal.player_id = player_id;
    store->balances.push_back(bal);
    return true;
}

bool LoyaltyPointStoreSystem::earnLP(const std::string& entity_id,
    const std::string& player_id, int amount) {
    auto* store = getComponentFor(entity_id);
    if (!store) return false;

    auto* bal = findBalance(store, player_id);
    if (!bal) return false;
    bal->balance += amount;
    bal->total_earned += amount;
    return true;
}

bool LoyaltyPointStoreSystem::purchaseItem(const std::string& entity_id,
    const std::string& player_id, const std::string& item_id) {
    auto* store = getComponentFor(entity_id);
    if (!store) return false;

    auto* bal = findBalance(store, player_id);
    if (!bal) return false;
    auto* item = findItem(store, item_id);
    if (!item || !item->in_stock) return false;

    if (bal->balance < item->lp_cost) return false;

    bal->balance -= item->lp_cost;
    bal->total_spent += item->lp_cost;
    item->times_purchased++;
    store->total_purchases++;
    store->isk_collected += item->isk_cost;
    return true;
}

int LoyaltyPointStoreSystem::getBalance(const std::string& entity_id,
    const std::string& player_id) const {
    auto* store = getComponentFor(entity_id);
    if (!store) return 0;
    auto* bal = findBalanceConst(store, player_id);
    return bal ? bal->balance : 0;
}

int LoyaltyPointStoreSystem::getItemCount(const std::string& entity_id) const {
    auto* store = getComponentFor(entity_id);
    return store ? static_cast<int>(store->items.size()) : 0;
}

int LoyaltyPointStoreSystem::getPlayerCount(const std::string& entity_id) const {
    auto* store = getComponentFor(entity_id);
    return store ? static_cast<int>(store->balances.size()) : 0;
}

int LoyaltyPointStoreSystem::getTotalPurchases(const std::string& entity_id) const {
    auto* store = getComponentFor(entity_id);
    return store ? store->total_purchases : 0;
}

float LoyaltyPointStoreSystem::getTotalISKCollected(const std::string& entity_id) const {
    auto* store = getComponentFor(entity_id);
    return store ? store->isk_collected : 0.0f;
}

int LoyaltyPointStoreSystem::getItemsByCategory(const std::string& entity_id,
    const std::string& category) const {
    auto* store = getComponentFor(entity_id);
    if (!store) return 0;

    int count = 0;
    for (const auto& item : store->items) {
        if (item.category == category) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
