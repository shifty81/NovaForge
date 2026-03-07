// Tests for: Insurance Claim System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/insurance_claim_system.h"

using namespace atlas;

// ==================== Insurance Claim System Tests ====================

static void testInsuranceClaimCreate() {
    std::cout << "\n=== InsuranceClaim: Create ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", "frigate_01", "player1"), "Init succeeds");
    assertTrue(sys.getTierCount("ship1") == 0, "No tiers initially");
    assertTrue(sys.getPolicyState("ship1") == "Uninsured", "State is Uninsured");
    assertTrue(sys.getActiveTier("ship1") == "", "No active tier");
    assertTrue(sys.getActivePayout("ship1") == 0.0, "No payout");
    assertTrue(sys.getTimeRemaining("ship1") == 0.0f, "No time remaining");
    assertTrue(sys.getTotalPremiumsPaid("ship1") == 0.0, "No premiums paid");
    assertTrue(sys.getTotalPayoutsReceived("ship1") == 0.0, "No payouts received");
    assertTrue(sys.getTotalClaims("ship1") == 0, "No claims");
    assertTrue(sys.getTotalPoliciesPurchased("ship1") == 0, "No policies purchased");
    assertTrue(sys.getTotalPoliciesExpired("ship1") == 0, "No policies expired");
}

static void testInsuranceClaimAddTiers() {
    std::cout << "\n=== InsuranceClaim: AddTiers ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    assertTrue(sys.addTier("ship1", "Basic", 1000.0, 5000.0, 3600.0f), "Add Basic tier");
    assertTrue(sys.addTier("ship1", "Standard", 2500.0, 12000.0, 7200.0f), "Add Standard tier");
    assertTrue(sys.addTier("ship1", "Platinum", 5000.0, 30000.0, 14400.0f), "Add Platinum tier");
    assertTrue(sys.getTierCount("ship1") == 3, "3 tiers added");
}

static void testInsuranceClaimDuplicateTier() {
    std::cout << "\n=== InsuranceClaim: DuplicateTier ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Basic", 1000.0, 5000.0, 3600.0f);
    assertTrue(!sys.addTier("ship1", "Basic", 2000.0, 10000.0, 7200.0f), "Duplicate tier rejected");
    assertTrue(sys.getTierCount("ship1") == 1, "Still 1 tier");
}

static void testInsuranceClaimPurchase() {
    std::cout << "\n=== InsuranceClaim: Purchase ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Basic", 1000.0, 5000.0, 3600.0f);
    sys.addTier("ship1", "Platinum", 5000.0, 30000.0, 14400.0f);

    assertTrue(sys.purchasePolicy("ship1", "Basic"), "Purchase Basic");
    assertTrue(sys.getPolicyState("ship1") == "Active", "State is Active");
    assertTrue(sys.getActiveTier("ship1") == "Basic", "Active tier is Basic");
    assertTrue(sys.getActivePayout("ship1") == 5000.0, "Payout is 5000");
    assertTrue(approxEqual(sys.getTimeRemaining("ship1"), 3600.0f), "Time remaining ~3600");
    assertTrue(sys.getTotalPremiumsPaid("ship1") == 1000.0, "Premium paid 1000");
    assertTrue(sys.getTotalPoliciesPurchased("ship1") == 1, "1 policy purchased");
}

static void testInsuranceClaimCannotDoubleInsure() {
    std::cout << "\n=== InsuranceClaim: CannotDoubleInsure ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Basic", 1000.0, 5000.0, 3600.0f);
    sys.addTier("ship1", "Platinum", 5000.0, 30000.0, 14400.0f);
    sys.purchasePolicy("ship1", "Basic");
    assertTrue(!sys.purchasePolicy("ship1", "Platinum"), "Cannot double-insure");
    assertTrue(sys.getActiveTier("ship1") == "Basic", "Still Basic");
}

static void testInsuranceClaimFileClaim() {
    std::cout << "\n=== InsuranceClaim: FileClaim ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Platinum", 5000.0, 30000.0, 14400.0f);
    sys.purchasePolicy("ship1", "Platinum");

    assertTrue(sys.fileClaim("ship1"), "File claim succeeds");
    assertTrue(sys.getPolicyState("ship1") == "ClaimPaid", "State is ClaimPaid");
    assertTrue(sys.getTotalPayoutsReceived("ship1") == 30000.0, "Payout received 30000");
    assertTrue(sys.getTotalClaims("ship1") == 1, "1 claim filed");
}

static void testInsuranceClaimCannotClaimUninsured() {
    std::cout << "\n=== InsuranceClaim: CannotClaimUninsured ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    assertTrue(!sys.fileClaim("ship1"), "Cannot claim without policy");
}

static void testInsuranceClaimCancel() {
    std::cout << "\n=== InsuranceClaim: Cancel ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Basic", 1000.0, 5000.0, 3600.0f);
    sys.purchasePolicy("ship1", "Basic");
    assertTrue(sys.cancelPolicy("ship1"), "Cancel policy succeeds");
    assertTrue(sys.getPolicyState("ship1") == "Uninsured", "State is Uninsured after cancel");
    assertTrue(sys.getActiveTier("ship1") == "", "No active tier after cancel");
    assertTrue(sys.getActivePayout("ship1") == 0.0, "No payout after cancel");
}

static void testInsuranceClaimExpiry() {
    std::cout << "\n=== InsuranceClaim: Expiry ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", "player1");
    sys.addTier("ship1", "Basic", 1000.0, 5000.0, 100.0f);
    sys.purchasePolicy("ship1", "Basic");

    sys.update(50.0f);
    assertTrue(sys.getPolicyState("ship1") == "Active", "Still Active at 50s");
    assertTrue(approxEqual(sys.getTimeRemaining("ship1"), 50.0f), "50s remaining");

    sys.update(60.0f); // 110s total, past 100s duration
    assertTrue(sys.getPolicyState("ship1") == "Expired", "Expired after duration");
    assertTrue(sys.getTotalPoliciesExpired("ship1") == 1, "1 policy expired");
}

static void testInsuranceClaimMissing() {
    std::cout << "\n=== InsuranceClaim: Missing ===" << std::endl;
    ecs::World world;
    systems::InsuranceClaimSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "ship", "owner"), "Init fails on missing");
    assertTrue(!sys.addTier("nonexistent", "Basic", 100.0, 500.0, 60.0f), "AddTier fails on missing");
    assertTrue(!sys.purchasePolicy("nonexistent", "Basic"), "Purchase fails on missing");
    assertTrue(!sys.fileClaim("nonexistent"), "FileClaim fails on missing");
    assertTrue(!sys.cancelPolicy("nonexistent"), "Cancel fails on missing");
    assertTrue(sys.getTierCount("nonexistent") == 0, "0 tiers on missing");
    assertTrue(sys.getPolicyState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(sys.getActiveTier("nonexistent") == "", "Empty tier on missing");
    assertTrue(sys.getActivePayout("nonexistent") == 0.0, "0 payout on missing");
    assertTrue(sys.getTimeRemaining("nonexistent") == 0.0f, "0 time on missing");
    assertTrue(sys.getTotalPremiumsPaid("nonexistent") == 0.0, "0 premiums on missing");
    assertTrue(sys.getTotalPayoutsReceived("nonexistent") == 0.0, "0 payouts on missing");
    assertTrue(sys.getTotalClaims("nonexistent") == 0, "0 claims on missing");
    assertTrue(sys.getTotalPoliciesPurchased("nonexistent") == 0, "0 purchased on missing");
    assertTrue(sys.getTotalPoliciesExpired("nonexistent") == 0, "0 expired on missing");
}


void run_insurance_claim_system_tests() {
    testInsuranceClaimCreate();
    testInsuranceClaimAddTiers();
    testInsuranceClaimDuplicateTier();
    testInsuranceClaimPurchase();
    testInsuranceClaimCannotDoubleInsure();
    testInsuranceClaimFileClaim();
    testInsuranceClaimCannotClaimUninsured();
    testInsuranceClaimCancel();
    testInsuranceClaimExpiry();
    testInsuranceClaimMissing();
}
