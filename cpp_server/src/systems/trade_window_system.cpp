#include "systems/trade_window_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TradeWindowSystem::TradeWindowSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void TradeWindowSystem::updateComponent(ecs::Entity& /*entity*/,
                                         components::TradeWindow& /*comp*/,
                                         float /*delta_time*/) {
    // Trade window is driven entirely by explicit method calls;
    // no per-frame logic is required.
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool TradeWindowSystem::initialize(const std::string& entity_id,
                                    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || owner_id.empty()) return false;
    auto comp = std::make_unique<components::TradeWindow>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Trade control
// ---------------------------------------------------------------------------

bool TradeWindowSystem::openTrade(const std::string& entity_id,
                                   const std::string& partner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Idle) return false;
    if (partner_id.empty()) return false;
    comp->partner_id        = partner_id;
    comp->state             = components::TradeWindow::TradeState::Open;
    comp->owner_confirmed   = false;
    comp->partner_confirmed = false;
    comp->my_offers.clear();
    return true;
}

bool TradeWindowSystem::cancelTrade(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    comp->state = components::TradeWindow::TradeState::Cancelled;
    comp->my_offers.clear();
    comp->owner_confirmed   = false;
    comp->partner_confirmed = false;
    return true;
}

bool TradeWindowSystem::confirmTrade(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    comp->owner_confirmed = true;
    return true;
}

bool TradeWindowSystem::setPartnerConfirmed(const std::string& entity_id,
                                             bool confirmed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    comp->partner_confirmed = confirmed;
    return true;
}

bool TradeWindowSystem::completeTrade(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    if (!comp->owner_confirmed || !comp->partner_confirmed) return false;
    comp->state = components::TradeWindow::TradeState::Complete;
    comp->total_trades++;
    return true;
}

// ---------------------------------------------------------------------------
// Offer management
// ---------------------------------------------------------------------------

bool TradeWindowSystem::addOffer(const std::string& entity_id,
                                  const std::string& item_id,
                                  const std::string& item_name,
                                  int quantity,
                                  float unit_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    if (comp->owner_confirmed) return false;  // cannot modify after confirm
    if (item_id.empty() || quantity <= 0) return false;
    if (static_cast<int>(comp->my_offers.size()) >= comp->max_offers) return false;

    for (const auto& offer : comp->my_offers) {
        if (offer.item_id == item_id) return false;
    }

    components::TradeWindow::TradeOffer offer;
    offer.item_id    = item_id;
    offer.item_name  = item_name;
    offer.quantity   = quantity;
    offer.unit_value = unit_value;
    comp->my_offers.push_back(offer);
    return true;
}

bool TradeWindowSystem::removeOffer(const std::string& entity_id,
                                     const std::string& item_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::TradeWindow::TradeState::Open) return false;
    if (comp->owner_confirmed) return false;  // cannot modify after confirm
    auto it = std::find_if(comp->my_offers.begin(), comp->my_offers.end(),
        [&](const components::TradeWindow::TradeOffer& o) {
            return o.item_id == item_id;
        });
    if (it == comp->my_offers.end()) return false;
    comp->my_offers.erase(it);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::TradeWindow::TradeState
TradeWindowSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->state : components::TradeWindow::TradeState::Idle;
}

bool TradeWindowSystem::isTradeOpen(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->state == components::TradeWindow::TradeState::Open;
}

int TradeWindowSystem::getOfferCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->my_offers.size()) : 0;
}

float TradeWindowSystem::getTotalOfferValue(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& offer : comp->my_offers) {
        total += offer.unit_value * static_cast<float>(offer.quantity);
    }
    return total;
}

int TradeWindowSystem::getTotalTrades(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_trades : 0;
}

std::string TradeWindowSystem::getPartnerId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->partner_id : std::string();
}

} // namespace systems
} // namespace atlas
