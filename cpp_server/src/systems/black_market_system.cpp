#include "systems/black_market_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

BlackMarketSystem::BlackMarketSystem(ecs::World* world)
    : System(world) {
}

void BlackMarketSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (!bm) continue;

        // Update expiry timers and remove expired listings
        for (auto it = bm->listings.begin(); it != bm->listings.end(); ) {
            it->expiry_timer += delta_time;
            if (it->expiry_timer >= it->max_expiry) {
                it = bm->listings.erase(it);
            } else {
                ++it;
            }
        }

        // Track listing refresh timer
        bm->listing_refresh_timer += delta_time;
        if (bm->listing_refresh_timer >= bm->listing_refresh_interval) {
            bm->listing_refresh_timer = 0.0f;
        }
    }
}

void BlackMarketSystem::addListing(const std::string& market_id, const std::string& item_id,
                                   const std::string& seller_id, float price, int quantity,
                                   bool contraband, float risk) {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (bm) bm->addListing(item_id, seller_id, price, quantity, contraband, risk);
    }
}

bool BlackMarketSystem::purchaseItem(const std::string& market_id, const std::string& item_id,
                                     const std::string& /*buyer_id*/) {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (!bm) continue;
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
    return false;
}

int BlackMarketSystem::getListingCount(const std::string& market_id) const {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (bm) return bm->getListingCount();
    }
    return 0;
}

float BlackMarketSystem::getDetectionChance(const std::string& market_id) const {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (bm) return bm->detection_chance_base * bm->security_level;
    }
    return 0.0f;
}

std::vector<std::string> BlackMarketSystem::getAvailableItems(const std::string& market_id) const {
    std::vector<std::string> items;
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (!bm) continue;
        for (const auto& l : bm->listings) {
            items.push_back(l.item_id);
        }
    }
    return items;
}

void BlackMarketSystem::setSecurityLevel(const std::string& market_id, float security) {
    auto entities = world_->getEntities<components::BlackMarket>();
    for (auto* entity : entities) {
        if (entity->getId() != market_id) continue;
        auto* bm = entity->getComponent<components::BlackMarket>();
        if (bm) bm->security_level = security;
    }
}

} // namespace systems
} // namespace atlas
