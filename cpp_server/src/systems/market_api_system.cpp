#include "systems/market_api_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MarketApiSystem::MarketApiSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void MarketApiSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::MarketApiState& comp,
                                      float delta_time) {
    if (!comp.active) return;
    comp.elapsed   += delta_time;
    comp.push_timer += delta_time;

    if (comp.push_timer >= comp.price_push_interval) {
        comp.push_timer = 0.0f;
        comp.total_pushes++;

        // Update push counters for all active subscriptions
        for (auto& sub : comp.subscriptions) {
            sub.snapshot_count++;
            sub.last_push = comp.elapsed;
        }
    }

    // Auto-fulfil pending requests (simulation: mark fulfilled next tick)
    for (auto& req : comp.pending_requests) {
        if (!req.fulfilled) {
            req.fulfilled = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool MarketApiSystem::initialize(const std::string& entity_id,
                                  const std::string& region_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MarketApiState>();
    comp->region_id = region_id.empty() ? entity_id : region_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Subscriptions
// ---------------------------------------------------------------------------

bool MarketApiSystem::subscribe(const std::string& entity_id,
                                 const std::string& client_id,
                                 const std::string& item_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || client_id.empty() || item_type.empty()) return false;
    if (static_cast<int>(comp->subscriptions.size()) >= comp->max_subscriptions)
        return false;

    // Prevent duplicate subscription for same client+item
    for (const auto& s : comp->subscriptions) {
        if (s.client_id == client_id && s.item_type == item_type) return false;
    }

    components::MarketApiState::Subscription sub;
    sub.client_id = client_id;
    sub.item_type = item_type;
    comp->subscriptions.push_back(sub);
    return true;
}

bool MarketApiSystem::unsubscribe(const std::string& entity_id,
                                   const std::string& client_id,
                                   const std::string& item_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto& subs = comp->subscriptions;
    auto it = std::find_if(subs.begin(), subs.end(),
        [&](const components::MarketApiState::Subscription& s) {
            return s.client_id == client_id && s.item_type == item_type;
        });
    if (it == subs.end()) return false;
    subs.erase(it);
    return true;
}

bool MarketApiSystem::isSubscribed(const std::string& entity_id,
                                    const std::string& client_id,
                                    const std::string& item_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->subscriptions) {
        if (s.client_id == client_id && s.item_type == item_type) return true;
    }
    return false;
}

int MarketApiSystem::getSubscriptionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->subscriptions.size()) : 0;
}

// ---------------------------------------------------------------------------
// Client requests
// ---------------------------------------------------------------------------

bool MarketApiSystem::submitRequest(const std::string& entity_id,
                                     const std::string& client_id,
                                     const std::string& request_type,
                                     const std::string& item_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || client_id.empty() || request_type.empty()) return false;

    components::MarketApiState::ClientRequest req;
    req.client_id    = client_id;
    req.request_type = request_type;
    req.item_type    = item_type;
    req.received_at  = comp->elapsed;
    req.fulfilled    = false;
    comp->pending_requests.push_back(req);
    comp->total_requests++;
    return true;
}

int MarketApiSystem::getPendingRequestCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->pending_requests) {
        if (!r.fulfilled) count++;
    }
    return count;
}

int MarketApiSystem::getTotalRequests(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_requests : 0;
}

// ---------------------------------------------------------------------------
// Price history
// ---------------------------------------------------------------------------

bool MarketApiSystem::recordSnapshot(const std::string& entity_id,
                                      float best_buy, float best_sell,
                                      float volume) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    components::MarketApiState::PriceSnapshot snap;
    snap.best_buy  = best_buy;
    snap.best_sell = best_sell;
    snap.volume    = volume;
    snap.timestamp = comp->elapsed;
    comp->price_history.push_back(snap);

    // Trim to max history
    while (static_cast<int>(comp->price_history.size()) > comp->max_history) {
        comp->price_history.erase(comp->price_history.begin());
    }
    return true;
}

int MarketApiSystem::getHistoryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->price_history.size()) : 0;
}

int MarketApiSystem::getTotalPushes(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_pushes : 0;
}

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------

bool MarketApiSystem::setPushInterval(const std::string& entity_id,
                                       float interval) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || interval <= 0.0f) return false;
    comp->price_push_interval = interval;
    return true;
}

float MarketApiSystem::getPushInterval(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->price_push_interval : 0.0f;
}

} // namespace systems
} // namespace atlas
