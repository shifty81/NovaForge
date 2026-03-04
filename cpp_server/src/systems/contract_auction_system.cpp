#include "systems/contract_auction_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using CA = components::ContractAuction;
using Listing = components::ContractAuction::AuctionListing;

Listing* findListing(CA* ca, const std::string& listing_id) {
    for (auto& l : ca->listings) {
        if (l.listing_id == listing_id) return &l;
    }
    return nullptr;
}

const Listing* findListingConst(const CA* ca, const std::string& listing_id) {
    for (const auto& l : ca->listings) {
        if (l.listing_id == listing_id) return &l;
    }
    return nullptr;
}

} // anonymous namespace

ContractAuctionSystem::ContractAuctionSystem(ecs::World* world)
    : System(world) {}

void ContractAuctionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::ContractAuction>();
    for (auto* entity : entities) {
        auto* ca = entity->getComponent<components::ContractAuction>();
        if (!ca || !ca->active) continue;

        for (auto& listing : ca->listings) {
            if (listing.state != CA::AuctionState::Active) continue;

            listing.elapsed += delta_time;
            if (listing.elapsed >= listing.duration) {
                if (listing.bid_count > 0) {
                    listing.state = CA::AuctionState::Sold;
                    ca->total_sold++;
                    ca->total_revenue += listing.current_bid;
                } else {
                    listing.state = CA::AuctionState::Expired;
                    ca->total_expired++;
                }
            }
        }
    }
}

bool ContractAuctionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ContractAuction>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ContractAuctionSystem::createListing(const std::string& entity_id,
    const std::string& listing_id, const std::string& seller_id,
    const std::string& item_name, const std::string& category,
    float starting_price, float buyout_price, float duration) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return false;
    if (static_cast<int>(ca->listings.size()) >= ca->max_listings) return false;
    if (findListing(ca, listing_id)) return false;
    if (starting_price <= 0.0f) return false;

    Listing l;
    l.listing_id = listing_id;
    l.seller_id = seller_id;
    l.item_name = item_name;
    l.category = category;
    l.starting_price = starting_price;
    l.buyout_price = buyout_price;
    l.duration = duration;
    l.state = CA::AuctionState::Pending;
    ca->listings.push_back(l);
    return true;
}

bool ContractAuctionSystem::activateListing(const std::string& entity_id,
    const std::string& listing_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return false;

    auto* listing = findListing(ca, listing_id);
    if (!listing) return false;
    if (listing->state != CA::AuctionState::Pending) return false;
    listing->state = CA::AuctionState::Active;
    return true;
}

bool ContractAuctionSystem::placeBid(const std::string& entity_id,
    const std::string& listing_id, const std::string& bidder_id,
    float amount, float timestamp) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return false;

    auto* listing = findListing(ca, listing_id);
    if (!listing) return false;
    if (listing->state != CA::AuctionState::Active) return false;
    if (amount <= listing->current_bid) return false;
    if (amount < listing->starting_price) return false;

    CA::Bid bid;
    bid.bidder_id = bidder_id;
    bid.amount = amount;
    bid.timestamp = timestamp;
    listing->bid_history.push_back(bid);
    listing->current_bid = amount;
    listing->highest_bidder = bidder_id;
    listing->bid_count++;
    ca->total_bids++;
    return true;
}

bool ContractAuctionSystem::buyout(const std::string& entity_id,
    const std::string& listing_id, const std::string& buyer_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return false;

    auto* listing = findListing(ca, listing_id);
    if (!listing) return false;
    if (listing->state != CA::AuctionState::Active) return false;
    if (listing->buyout_price <= 0.0f) return false;

    listing->current_bid = listing->buyout_price;
    listing->highest_bidder = buyer_id;
    listing->state = CA::AuctionState::Sold;
    ca->total_sold++;
    ca->total_revenue += listing->buyout_price;
    return true;
}

bool ContractAuctionSystem::cancelListing(const std::string& entity_id,
    const std::string& listing_id, const std::string& seller_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return false;

    auto* listing = findListing(ca, listing_id);
    if (!listing) return false;
    if (listing->state != CA::AuctionState::Pending &&
        listing->state != CA::AuctionState::Active) return false;
    if (listing->seller_id != seller_id) return false;
    if (listing->bid_count > 0) return false; // can't cancel with bids

    listing->state = CA::AuctionState::Cancelled;
    return true;
}

int ContractAuctionSystem::getListingCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    return ca ? static_cast<int>(ca->listings.size()) : 0;
}

int ContractAuctionSystem::getActiveListingCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return 0;
    int count = 0;
    for (const auto& l : ca->listings) {
        if (l.state == CA::AuctionState::Active) count++;
    }
    return count;
}

int ContractAuctionSystem::getBidCount(const std::string& entity_id,
    const std::string& listing_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return 0;
    const auto* listing = findListingConst(ca, listing_id);
    return listing ? listing->bid_count : 0;
}

float ContractAuctionSystem::getCurrentBid(const std::string& entity_id,
    const std::string& listing_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return 0.0f;
    const auto* listing = findListingConst(ca, listing_id);
    return listing ? listing->current_bid : 0.0f;
}

int ContractAuctionSystem::getState(const std::string& entity_id,
    const std::string& listing_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    if (!ca) return 0;
    const auto* listing = findListingConst(ca, listing_id);
    return listing ? static_cast<int>(listing->state) : 0;
}

int ContractAuctionSystem::getTotalSold(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    return ca ? ca->total_sold : 0;
}

int ContractAuctionSystem::getTotalExpired(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ca = entity->getComponent<components::ContractAuction>();
    return ca ? ca->total_expired : 0;
}

float ContractAuctionSystem::getTotalRevenue(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ca = entity->getComponent<components::ContractAuction>();
    return ca ? ca->total_revenue : 0.0f;
}

} // namespace systems
} // namespace atlas
