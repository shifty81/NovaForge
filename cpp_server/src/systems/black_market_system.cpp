#include "systems/black_market_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

BlackMarketSystem::BlackMarketSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void BlackMarketSystem::updateComponent(ecs::Entity& /*entity*/, components::BlackMarket& comp, float delta_time) {
    // Update expiry timers and remove expired listings
    for (auto it = comp.listings.begin(); it != comp.listings.end(); ) {
        it->expiry_timer += delta_time;
        if (it->expiry_timer >= it->max_expiry) {
            it = comp.listings.erase(it);
        } else {
            ++it;
        }
    }

    // Track listing refresh timer
    comp.listing_refresh_timer += delta_time;
    if (comp.listing_refresh_timer >= comp.listing_refresh_interval) {
        comp.listing_refresh_timer = 0.0f;
    }
}

void BlackMarketSystem::addListing(const std::string& market_id, const std::string& item_id,
                                   const std::string& seller_id, float price, int quantity,
                                   bool contraband, float risk) {
    auto* bm = getComponentFor(market_id);
    if (bm) bm->addListing(item_id, seller_id, price, quantity, contraband, risk);
}

bool BlackMarketSystem::purchaseItem(const std::string& market_id, const std::string& item_id,
                                     const std::string& /*buyer_id*/) {
    auto* bm = getComponentFor(market_id);
    if (!bm) return false;
    auto* listing = bm->findListing(item_id);
    if (!listing || listing->quantity <= 0) return false;
    listing->quantity--;
    if (listing->quantity <= 0) {
        bm->listings.erase(
            std::remove_if(bm->listings.begin(), bm->listings.end(),
                [&](const components::BlackMarket::Listing& l) {
                    return l.item_id == item_id && l.quantity <= 0;
                }),
            bm->listings.end());
    }
    return true;
}

int BlackMarketSystem::getListingCount(const std::string& market_id) const {
    const auto* bm = getComponentFor(market_id);
    return bm ? bm->getListingCount() : 0;
}

float BlackMarketSystem::getDetectionChance(const std::string& market_id) const {
    const auto* bm = getComponentFor(market_id);
    return bm ? bm->detection_chance_base * bm->security_level : 0.0f;
}

std::vector<std::string> BlackMarketSystem::getAvailableItems(const std::string& market_id) const {
    std::vector<std::string> items;
    const auto* bm = getComponentFor(market_id);
    if (!bm) return items;
    for (const auto& l : bm->listings) {
        items.push_back(l.item_id);
    }
    return items;
}

void BlackMarketSystem::setSecurityLevel(const std::string& market_id, float security) {
    auto* bm = getComponentFor(market_id);
    if (bm) bm->security_level = security;
}

} // namespace systems
} // namespace atlas
