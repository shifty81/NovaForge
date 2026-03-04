// Tests for: Contract Auction System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/contract_auction_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Contract Auction System Tests ====================

static void testContractAuctionCreate() {
    std::cout << "\n=== ContractAuction: Create ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    assertTrue(sys.initialize("market1"), "Init succeeds");
    assertTrue(sys.getListingCount("market1") == 0, "No listings initially");
    assertTrue(sys.getActiveListingCount("market1") == 0, "No active listings");
    assertTrue(sys.getTotalSold("market1") == 0, "No sold");
    assertTrue(sys.getTotalExpired("market1") == 0, "No expired");
    assertTrue(approxEqual(sys.getTotalRevenue("market1"), 0.0f), "No revenue");
}

static void testContractAuctionCreateListing() {
    std::cout << "\n=== ContractAuction: CreateListing ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    assertTrue(sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship",
                                 100.0f, 500.0f, 3600.0f), "Create listing succeeds");
    assertTrue(sys.getListingCount("market1") == 1, "1 listing");
    assertTrue(sys.getState("market1", "lot1") == 0, "State is Pending");
    assertTrue(sys.createListing("market1", "lot2", "seller2", "Shield Booster", "Module",
                                 50.0f, 0.0f, 1800.0f), "Create second listing");
    assertTrue(sys.getListingCount("market1") == 2, "2 listings");
}

static void testContractAuctionDuplicate() {
    std::cout << "\n=== ContractAuction: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    assertTrue(!sys.createListing("market1", "lot1", "seller2", "Caracal", "Ship",
                                  200.0f, 800.0f, 3600.0f), "Duplicate listing rejected");
    assertTrue(!sys.createListing("market1", "lot3", "seller1", "Rifter", "Ship",
                                  0.0f, 500.0f, 3600.0f), "Zero starting price rejected");
    assertTrue(!sys.createListing("market1", "lot3", "seller1", "Rifter", "Ship",
                                  -10.0f, 500.0f, 3600.0f), "Negative price rejected");
}

static void testContractAuctionActivate() {
    std::cout << "\n=== ContractAuction: Activate ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    assertTrue(sys.activateListing("market1", "lot1"), "Activate succeeds");
    assertTrue(sys.getState("market1", "lot1") == 1, "State is Active");
    assertTrue(sys.getActiveListingCount("market1") == 1, "1 active listing");
    assertTrue(!sys.activateListing("market1", "lot1"), "Double activate rejected");
    assertTrue(!sys.activateListing("market1", "nonexistent"), "Activate missing fails");
}

static void testContractAuctionBid() {
    std::cout << "\n=== ContractAuction: Bid ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    sys.activateListing("market1", "lot1");

    assertTrue(!sys.placeBid("market1", "lot1", "buyer1", 50.0f, 1.0f),
               "Bid below starting price rejected");
    assertTrue(sys.placeBid("market1", "lot1", "buyer1", 150.0f, 1.0f),
               "Valid bid succeeds");
    assertTrue(approxEqual(sys.getCurrentBid("market1", "lot1"), 150.0f), "Current bid is 150");
    assertTrue(sys.getBidCount("market1", "lot1") == 1, "1 bid");
    assertTrue(!sys.placeBid("market1", "lot1", "buyer2", 100.0f, 2.0f),
               "Lower bid rejected");
    assertTrue(sys.placeBid("market1", "lot1", "buyer2", 200.0f, 2.0f),
               "Higher bid succeeds");
    assertTrue(approxEqual(sys.getCurrentBid("market1", "lot1"), 200.0f), "Current bid is 200");
    assertTrue(sys.getBidCount("market1", "lot1") == 2, "2 bids");
}

static void testContractAuctionBuyout() {
    std::cout << "\n=== ContractAuction: Buyout ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    sys.createListing("market1", "lot2", "seller2", "Caracal", "Ship", 200.0f, 0.0f, 3600.0f);
    sys.activateListing("market1", "lot1");
    sys.activateListing("market1", "lot2");

    assertTrue(sys.buyout("market1", "lot1", "buyer1"), "Buyout succeeds");
    assertTrue(sys.getState("market1", "lot1") == 2, "State is Sold");
    assertTrue(approxEqual(sys.getCurrentBid("market1", "lot1"), 500.0f), "Bid is buyout price");
    assertTrue(sys.getTotalSold("market1") == 1, "1 sold");
    assertTrue(approxEqual(sys.getTotalRevenue("market1"), 500.0f), "Revenue is 500");
    assertTrue(!sys.buyout("market1", "lot2", "buyer2"), "Buyout with 0 price fails");
}

static void testContractAuctionCancel() {
    std::cout << "\n=== ContractAuction: Cancel ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    assertTrue(sys.cancelListing("market1", "lot1", "seller1"), "Cancel pending succeeds");
    assertTrue(sys.getState("market1", "lot1") == 4, "State is Cancelled");
    assertTrue(!sys.cancelListing("market1", "lot1", "seller1"), "Double cancel fails");

    sys.createListing("market1", "lot2", "seller2", "Caracal", "Ship", 200.0f, 800.0f, 3600.0f);
    sys.activateListing("market1", "lot2");
    assertTrue(!sys.cancelListing("market1", "lot2", "seller1"), "Cancel by non-seller fails");
    assertTrue(sys.cancelListing("market1", "lot2", "seller2"), "Cancel active no-bids succeeds");

    sys.createListing("market1", "lot3", "seller3", "Moa", "Ship", 300.0f, 900.0f, 3600.0f);
    sys.activateListing("market1", "lot3");
    sys.placeBid("market1", "lot3", "buyer1", 350.0f, 1.0f);
    assertTrue(!sys.cancelListing("market1", "lot3", "seller3"), "Cancel with bids fails");
}

static void testContractAuctionExpiry() {
    std::cout << "\n=== ContractAuction: Expiry ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 10.0f);
    sys.createListing("market1", "lot2", "seller2", "Caracal", "Ship", 200.0f, 800.0f, 10.0f);
    sys.activateListing("market1", "lot1");
    sys.activateListing("market1", "lot2");

    sys.placeBid("market1", "lot1", "buyer1", 150.0f, 1.0f);

    sys.update(11.0f);
    assertTrue(sys.getState("market1", "lot1") == 2, "Lot1 sold (had bids)");
    assertTrue(sys.getState("market1", "lot2") == 3, "Lot2 expired (no bids)");
    assertTrue(sys.getTotalSold("market1") == 1, "1 sold");
    assertTrue(sys.getTotalExpired("market1") == 1, "1 expired");
    assertTrue(approxEqual(sys.getTotalRevenue("market1"), 150.0f), "Revenue from bid");
}

static void testContractAuctionMaxLimit() {
    std::cout << "\n=== ContractAuction: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    auto* entity = world.getEntity("market1");
    auto* ca = entity->getComponent<components::ContractAuction>();
    ca->max_listings = 2;

    sys.createListing("market1", "lot1", "seller1", "Rifter", "Ship", 100.0f, 500.0f, 3600.0f);
    sys.createListing("market1", "lot2", "seller2", "Caracal", "Ship", 200.0f, 800.0f, 3600.0f);
    assertTrue(!sys.createListing("market1", "lot3", "seller3", "Moa", "Ship",
                                  300.0f, 900.0f, 3600.0f), "Max listings enforced");
}

static void testContractAuctionMissing() {
    std::cout << "\n=== ContractAuction: Missing ===" << std::endl;
    ecs::World world;
    systems::ContractAuctionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.createListing("nonexistent", "lot1", "s1", "X", "Ship",
                                  100.0f, 500.0f, 3600.0f), "Create fails on missing");
    assertTrue(!sys.activateListing("nonexistent", "lot1"), "Activate fails on missing");
    assertTrue(!sys.placeBid("nonexistent", "lot1", "b1", 200.0f, 1.0f), "Bid fails on missing");
    assertTrue(!sys.buyout("nonexistent", "lot1", "b1"), "Buyout fails on missing");
    assertTrue(!sys.cancelListing("nonexistent", "lot1", "s1"), "Cancel fails on missing");
    assertTrue(sys.getListingCount("nonexistent") == 0, "0 listings on missing");
    assertTrue(sys.getActiveListingCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getBidCount("nonexistent", "lot1") == 0, "0 bids on missing");
    assertTrue(approxEqual(sys.getCurrentBid("nonexistent", "lot1"), 0.0f), "0 bid on missing");
    assertTrue(sys.getState("nonexistent", "lot1") == 0, "0 state on missing");
    assertTrue(sys.getTotalSold("nonexistent") == 0, "0 sold on missing");
    assertTrue(sys.getTotalExpired("nonexistent") == 0, "0 expired on missing");
    assertTrue(approxEqual(sys.getTotalRevenue("nonexistent"), 0.0f), "0 revenue on missing");
}


void run_contract_auction_system_tests() {
    testContractAuctionCreate();
    testContractAuctionCreateListing();
    testContractAuctionDuplicate();
    testContractAuctionActivate();
    testContractAuctionBid();
    testContractAuctionBuyout();
    testContractAuctionCancel();
    testContractAuctionExpiry();
    testContractAuctionMaxLimit();
    testContractAuctionMissing();
}
