#include "systems/market_watchlist_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MarketWatchlistSystem::MarketWatchlistSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void MarketWatchlistSystem::updateComponent(ecs::Entity& /*entity*/,
                                             components::MarketWatchlist& comp,
                                             float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool MarketWatchlistSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MarketWatchlist>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Watchlist management
// ---------------------------------------------------------------------------

bool MarketWatchlistSystem::addEntry(const std::string& entity_id,
                                      const std::string& entry_id,
                                      const std::string& item_name,
                                      float current_price) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) return false;

    for (const auto& e : comp->entries) {
        if (e.id == entry_id) return false;
    }

    components::MarketWatchlist::WatchEntry entry;
    entry.id            = entry_id;
    entry.item_name     = item_name;
    entry.current_price = current_price;
    comp->entries.push_back(entry);
    return true;
}

bool MarketWatchlistSystem::removeEntry(const std::string& entity_id,
                                         const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->entries.begin(), comp->entries.end(),
        [&](const components::MarketWatchlist::WatchEntry& e) {
            return e.id == entry_id;
        });
    if (it == comp->entries.end()) return false;
    comp->entries.erase(it);
    return true;
}

bool MarketWatchlistSystem::clearWatchlist(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Alert thresholds
// ---------------------------------------------------------------------------

bool MarketWatchlistSystem::setBuyThreshold(const std::string& entity_id,
                                             const std::string& entry_id,
                                             float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold <= 0.0f) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.buy_threshold  = threshold;
            e.buy_alert_set  = true;
            e.buy_alert_fired = false;  // reset alert on threshold change
            return true;
        }
    }
    return false;
}

bool MarketWatchlistSystem::setSellThreshold(const std::string& entity_id,
                                              const std::string& entry_id,
                                              float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold <= 0.0f) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.sell_threshold  = threshold;
            e.sell_alert_set  = true;
            e.sell_alert_fired = false;  // reset alert on threshold change
            return true;
        }
    }
    return false;
}

bool MarketWatchlistSystem::clearBuyThreshold(const std::string& entity_id,
                                               const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.buy_threshold  = 0.0f;
            e.buy_alert_set  = false;
            e.buy_alert_fired = false;
            return true;
        }
    }
    return false;
}

bool MarketWatchlistSystem::clearSellThreshold(const std::string& entity_id,
                                                const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.sell_threshold  = 0.0f;
            e.sell_alert_set  = false;
            e.sell_alert_fired = false;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Price updates
// ---------------------------------------------------------------------------

bool MarketWatchlistSystem::updatePrice(const std::string& entity_id,
                                         const std::string& entry_id,
                                         float new_price) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.current_price = new_price;
            // Check buy threshold: fire if price drops below threshold
            if (e.buy_alert_set && !e.buy_alert_fired &&
                new_price <= e.buy_threshold) {
                e.buy_alert_fired = true;
                comp->total_alerts_fired++;
            }
            // Check sell threshold: fire if price rises above threshold
            if (e.sell_alert_set && !e.sell_alert_fired &&
                new_price >= e.sell_threshold) {
                e.sell_alert_fired = true;
                comp->total_alerts_fired++;
            }
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Alert acknowledgement
// ---------------------------------------------------------------------------

bool MarketWatchlistSystem::acknowledgeAlert(const std::string& entity_id,
                                              const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.id == entry_id) {
            e.buy_alert_fired  = false;
            e.sell_alert_fired = false;
            return true;
        }
    }
    return false;
}

bool MarketWatchlistSystem::acknowledgeAllAlerts(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        e.buy_alert_fired  = false;
        e.sell_alert_fired = false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int MarketWatchlistSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

float MarketWatchlistSystem::getCurrentPrice(const std::string& entity_id,
                                              const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.id == entry_id) return e.current_price;
    }
    return 0.0f;
}

bool MarketWatchlistSystem::hasBuyAlert(const std::string& entity_id,
                                         const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.id == entry_id) return e.buy_alert_fired;
    }
    return false;
}

bool MarketWatchlistSystem::hasSellAlert(const std::string& entity_id,
                                          const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.id == entry_id) return e.sell_alert_fired;
    }
    return false;
}

int MarketWatchlistSystem::getPendingAlertCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.buy_alert_fired)  count++;
        if (e.sell_alert_fired) count++;
    }
    return count;
}

int MarketWatchlistSystem::getTotalAlertsFired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_alerts_fired : 0;
}

bool MarketWatchlistSystem::hasEntry(const std::string& entity_id,
                                      const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.id == entry_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
