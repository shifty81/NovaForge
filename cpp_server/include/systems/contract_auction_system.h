#ifndef NOVAFORGE_SYSTEMS_CONTRACT_AUCTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONTRACT_AUCTION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Auction-based contract system for marketplace bidding
 *
 * Manages auction listings with bids, buyout prices, time-based expiry,
 * seller/buyer tracking, and bid history.
 */
class ContractAuctionSystem : public ecs::System {
public:
    explicit ContractAuctionSystem(ecs::World* world);
    ~ContractAuctionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ContractAuctionSystem"; }

    bool initialize(const std::string& entity_id);
    bool createListing(const std::string& entity_id, const std::string& listing_id,
                       const std::string& seller_id, const std::string& item_name,
                       const std::string& category, float starting_price,
                       float buyout_price, float duration);
    bool activateListing(const std::string& entity_id, const std::string& listing_id);
    bool placeBid(const std::string& entity_id, const std::string& listing_id,
                  const std::string& bidder_id, float amount, float timestamp);
    bool buyout(const std::string& entity_id, const std::string& listing_id,
                const std::string& buyer_id);
    bool cancelListing(const std::string& entity_id, const std::string& listing_id,
                       const std::string& seller_id);

    int getListingCount(const std::string& entity_id) const;
    int getActiveListingCount(const std::string& entity_id) const;
    int getBidCount(const std::string& entity_id, const std::string& listing_id) const;
    float getCurrentBid(const std::string& entity_id, const std::string& listing_id) const;
    int getState(const std::string& entity_id, const std::string& listing_id) const;
    int getTotalSold(const std::string& entity_id) const;
    int getTotalExpired(const std::string& entity_id) const;
    float getTotalRevenue(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONTRACT_AUCTION_SYSTEM_H
